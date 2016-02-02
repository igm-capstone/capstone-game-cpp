#include "stdafx.h"
#include "NetworkServer.h"

#include "ScareTacticsApplication.h"
#include "Scenes/Level00.h"

bool NetworkServer::Init(void) 
{
	WSADATA wsaData;

	mListenSocket = INVALID_SOCKET;
	mClientSocket = INVALID_SOCKET;

	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) {
		printf("WSAStartup failed: %d\n", ret);
		return false;
	}

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	ret = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (ret != 0) {
		printf("getaddrinfo failed: %d\n", ret);
		WSACleanup();
		return false;
	}

	mListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (mListenSocket == INVALID_SOCKET) {
		printf("socket failed: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	u_long iMode = 1;
	ret = ioctlsocket(mListenSocket, FIONBIO, &iMode);		// nonblocking mode
	if (ret == SOCKET_ERROR) {
		printf("ioctlsocket failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(mListenSocket);
		WSACleanup();
		return false;
	}

	ret = ::bind(mListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (ret == SOCKET_ERROR) {
		printf("bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(mListenSocket);
		WSACleanup();
		return false;
	}

	ret = listen(mListenSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		printf("listen: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(mListenSocket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);
	return true;
}


void NetworkServer::Shutdown(void)
{
	closesocket(mListenSocket);
	WSACleanup();
}

// accept new connections
void NetworkServer::CheckForNewClients()
{
	if (mClientList.size() > MAX_CLIENTS) return;

	mClientSocket = accept(mListenSocket, NULL, NULL);
	if (mClientSocket == INVALID_SOCKET) return;
	
	char value = 1; //disable nagle on the client's socket
	setsockopt(mClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
	
	mClientList[mClientID] = mClientSocket;
	mClientID++;

	printf("client %d connected\n", mClientID);
}

int NetworkServer::ReceiveData(unsigned int clientID, char* recvBuf)
{
	if (mClientList.find(clientID) != mClientList.end())
	{
		SOCKET currentSocket = mClientList[clientID];
		int ret = recv(currentSocket, recvBuf, MAX_DATA_SIZE, 0);

		if (ret == 0)
		{
			printf("client %d disconnected\n", clientID);
			closesocket(currentSocket);
		}

		return ret;
	}

	return 0;
}

void NetworkServer::SendToAll(Packet* p)
{
	SOCKET currentSocket;
	int iSendResult;
	p->Serialize(mPacketData);

	for each (auto client in mClientList)
	{
		currentSocket = client.second;
		iSendResult = send(client.second, mPacketData, sizeof(mPacketData), 0);

		if (iSendResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(currentSocket);
		}
	}
}

void NetworkServer::Retransmit(int originalSender, Packet* p)
{
	SOCKET currentSocket;
	int iSendResult;
	p->Serialize(mPacketData);

	for each (auto client in mClientList)
	{
		if (client.first == originalSender) continue;
		currentSocket = client.second;
		iSendResult = send(client.second, mPacketData, sizeof(mPacketData), 0);

		if (iSendResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(currentSocket);
		}
	}
}

void NetworkServer::Send(int id, Packet* p)
{
	int iSendResult;
	p->Serialize(mPacketData);

	iSendResult = send(mClientList[id], mPacketData, sizeof(mPacketData), 0);

	if (iSendResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(mClientList[id]);
	}

}

void NetworkServer::ReceiveFromClients()
{
	Packet packet;

	for each (auto client in mClientList)
	{
		int dataLength = ReceiveData(client.first, mNetworkData);

		if (dataLength <= 0) continue;

		for (int i = 0; i < dataLength; i += sizeof(Packet))
		{
			packet.Deserialize(&(mNetworkData[i]));
			switch (packet.Type) {
				case INIT_CONNECTION:
					{
						//Inform the client ID
						Packet p(PacketTypes::SET_CLIENT_ID);
						p.ClientID = client.first;
						Send(client.first, &p);

						//Spawn on the server
						Application::SharedInstance().GetCurrentScene()->CmdSpawnNewExplorer(client.first);
					}
					break;
				case SYNC_TRANSFORM:
					Retransmit(client.first, &packet);
					Application::SharedInstance().GetCurrentScene()->SyncTransform(packet.UUID, packet.Position);
					break;
				default:
					printf("error in packet types\n");
					break;
			}
		}
	}
}


void NetworkServer::Update()
{
	CheckForNewClients();

	ReceiveFromClients();
}
