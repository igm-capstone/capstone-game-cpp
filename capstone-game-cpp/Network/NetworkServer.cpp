#include "stdafx.h"
#include "NetworkServer.h"

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
	
	mClientID++;
	mClientList[mClientID] = mClientSocket;
	
	printf("client %d connected\n", mClientID);
}

int NetworkServer::ReceiveData(unsigned int clientID, char* recvBuf)
{
	if (mClientList.find(clientID) != mClientList.end())
	{
		SOCKET currentSocket = mClientList[clientID];
		int ret = recv(currentSocket, recvBuf, MAX_PACKET_SIZE, 0);

		if (ret == 0)
		{
			printf("client %d disconnected\n", clientID);
			closesocket(currentSocket);
			mClientList.erase(clientID);
		}

		return ret;
	}

	return 0;
}

void NetworkServer::SendToAll(char* buf, int bufSize)
{
	SOCKET currentSocket;
	int iSendResult;

	for each (auto client in mClientList)
	{
		currentSocket = client.second;
		iSendResult = send(client.second, buf, bufSize, 0);

		if (iSendResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(currentSocket);
		}
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
					printf("server received init packet from client\n");
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
