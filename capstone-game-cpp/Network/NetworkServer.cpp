#include "stdafx.h"
#include "NetworkServer.h"
#include <trace.h>
#include <SceneObjects/Explorer.h>
#include <Components/NetworkID.h>

ExplorerType gSpawnOrder[] = { TRAPMASTER, HEALER, SPRINTER };

bool NetworkServer::Init(void)
{
	WSADATA wsaData;

	mListenSocket = INVALID_SOCKET;
	mClientSocket = INVALID_SOCKET;

	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) {
		TRACE_LOG("WSAStartup failed:" << ret);
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
		TRACE_LOG("getaddrinfo failed:" << ret);
		WSACleanup();
		return false;
	}

	mListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (mListenSocket == INVALID_SOCKET) {
		TRACE_LOG("socket failed:" << WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	u_long iMode = 1;
	ret = ioctlsocket(mListenSocket, FIONBIO, &iMode);		// nonblocking mode
	if (ret == SOCKET_ERROR) {
		TRACE_LOG("ioctlsocket failed:" << WSAGetLastError());
		freeaddrinfo(result);
		closesocket(mListenSocket);
		WSACleanup();
		return false;
	}

	ret = ::bind(mListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (ret == SOCKET_ERROR) {
		TRACE_LOG("bind failed:" << WSAGetLastError());
		freeaddrinfo(result);
		closesocket(mListenSocket);
		WSACleanup();
		return false;
	}

	ret = listen(mListenSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		TRACE_LOG("listen:" << WSAGetLastError());
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

void NetworkServer::CheckForNewClients()
{
	if (mClientCount == MAX_EXPLORERS) return;

	mClientSocket = accept(mListenSocket, NULL, NULL);
	if (mClientSocket == INVALID_SOCKET) return;

	char value = 1; //disable nagle on the client's socket
	setsockopt(mClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

	unsigned int i;
	for (auto type : gSpawnOrder) {
		i = type;
		if (!mClientList[i]) break;
	}
	
	Packet p(PacketTypes::SET_CLIENT_ID);
	p.ClientID = i;
	p.Serialize(mPacketData);
	auto ret = send(mClientSocket, mPacketData, sizeof(mPacketData), 0);
	if (ret > 0) { 
		mClientList[i] = mClientSocket; 
		mClientCount++;
		TRACE_LOG("client connected " << i);
	}
	else
	{
		closesocket(mClientSocket);
		TRACE_LOG("client lost");
	}
}

void NetworkServer::RemoveClient(const unsigned clientID)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == clientID) {
			Explorer* e = static_cast<Explorer*>(netID.mSceneObject);
			Factory<Explorer>::Destroy(e);
		}
	}

	closesocket(mClientList[clientID]);
	mClientList[clientID] = 0;
	mClientCount--;

	Packet p(DISCONNECT);
	p.UUID = clientID;
	SendToAll(&p);
}

void NetworkServer::SendToAll(Packet* p)
{
	Retransmit(-1, p);
}

void NetworkServer::Retransmit(int originalSender, Packet* p)
{
	SOCKET currentSocket;
	int iSendResult;
	p->Serialize(mPacketData);

	for (int clientID = 1; clientID <= MAX_EXPLORERS; clientID++)
	{
		currentSocket = mClientList[clientID];

		if (!currentSocket) continue;
		if (clientID == originalSender) continue;

		iSendResult = send(currentSocket, mPacketData, sizeof(mPacketData), 0);

		if (iSendResult <= 0)
		{
			TRACE_LOG("Retransmit failed with error: " << WSAGetLastError() << ". Disconnecting client: " << clientID);
			RemoveClient(clientID);
		}
		//else TRACE_LOG("Sent " << p->Type << " to " << clientID);
	}
}

void NetworkServer::Send(int id, Packet* p)
{
	int iSendResult;
	p->Serialize(mPacketData);

	iSendResult = send(mClientList[id], mPacketData, sizeof(mPacketData), 0);

	if (iSendResult <= 0)
	{
		TRACE_LOG("Retransmit failed with error: " << WSAGetLastError() << ". Disconnecting client: " << id);
		RemoveClient(id);
	}
	//else TRACE_LOG("Sent " << p->Type << " to " << id);
}

void NetworkServer::ReceiveFromClients()
{
	Packet packet;
	SOCKET currentSocket;

	for (int clientID = 1; clientID <= MAX_EXPLORERS; clientID++)
	{
		currentSocket = mClientList[clientID];
		if (!currentSocket) continue;

		int dataLength = recv(currentSocket, mNetworkData, MAX_DATA_SIZE, 0);

		if (dataLength == 0 || (dataLength == -1 && WSAGetLastError() == WSAECONNRESET))
		{
			TRACE_LOG("client disconnected: " << clientID);
			RemoveClient(clientID);
		}
		else if (dataLength > 0) {
			for (int i = 0; i < dataLength; i += sizeof(Packet))
			{
				packet.Deserialize(&(mNetworkData[i]));
				switch (packet.Type) {
				case INIT_CONNECTION:
					NetworkCmd::SpawnNewExplorer(clientID);
					break;
				case SYNC_TRANSFORM:
					Retransmit(clientID, &packet);
					NetworkRpc::SyncTransform(packet.UUID,
						packet.AsTransform.Position, packet.AsTransform.Rotation);
					break;
				case SYNC_HEALTH:
					Retransmit(clientID, &packet);
					NetworkRpc::SyncHealth(packet.UUID, packet.AsFloatArray[0], packet.AsFloatArray[1], packet.AsFloatArray[2]);
					break;
				case SYNC_ANIMATION:
					Retransmit(clientID, &packet);
					NetworkRpc::SyncAnimation(packet.UUID, packet.AsAnimation.State, packet.AsAnimation.Command);
					break;
				case SPAWN_SKILL:
					NetworkCmd::SpawnNewSkill(packet.AsSkill.Type, packet.AsSkill.Position, packet.AsSkill.Duration, packet.AsSkill.TargetUUID);
					break;
				case INTERACT:
					Retransmit(clientID, &packet);
					NetworkRpc::Interact(packet.UUID);
					break;
				case READY:
					Retransmit(clientID, &packet);
					NetworkRpc::Ready(packet.ClientID, packet.AsBool);
					break;
				default:
					TRACE_LOG("error in packet types\n");
					break;
				}
			}
		}
	}
}


void NetworkServer::Update()
{
	CheckForNewClients();

	ReceiveFromClients();
}
