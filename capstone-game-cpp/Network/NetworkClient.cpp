#include "stdafx.h"
#include "NetworkClient.h"
#include <trace.h>

bool NetworkClient::Init(void)
{
	WSADATA wsaData;

	mConnectSocket = INVALID_SOCKET;

	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) {
		TRACE_LOG("WSAStartup failed" << ret);
		return false;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	ret = getaddrinfo(mIPAddress, DEFAULT_PORT, &hints, &result);
	if (ret != 0) {
		TRACE_LOG("getaddrinfo failed" << ret);
		WSACleanup();
		return false;
	}
	
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		mConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (mConnectSocket == INVALID_SOCKET) {
			TRACE_LOG("socket failed: " << WSAGetLastError());
			WSACleanup();
			return false;
		}

		ret = connect(mConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (ret == SOCKET_ERROR)
		{
			closesocket(mConnectSocket);
			mConnectSocket = INVALID_SOCKET;
			TRACE_LOG("The server is down... did not connect");
		}
	}

	freeaddrinfo(result);

	if (mConnectSocket == INVALID_SOCKET) {
		TRACE_LOG("Unable to connect to server!\n");
		WSACleanup();
		return false;
	}

	u_long iMode = 1;
	ret = ioctlsocket(mConnectSocket, FIONBIO, &iMode); //non-blocking mode
	if (ret == SOCKET_ERROR) {
		TRACE_LOG("ioctlsocket failed" << WSAGetLastError());
		closesocket(mConnectSocket);
		WSACleanup();
		return false;
	}

	fd_set readset;
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	do {
		FD_ZERO(&readset);
		FD_SET(mConnectSocket, &readset);
		ret = select(mConnectSocket + 1, &readset, NULL, NULL, &tv);
	} while (ret == -1 && errno == EINTR);
	if (ret <= 0) {
		TRACE_LOG("Connection wasnt accepted");
		closesocket(mConnectSocket);
		WSACleanup();
		return false;
	}

	char value = 1;
	setsockopt(mConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)); //disable nagle

	return true;
}

void NetworkClient::Shutdown(void)
{
	closesocket(mConnectSocket);
	WSACleanup();
}

int NetworkClient::ReceiveData(char * recvBuf)
{
	int ret = recv(mConnectSocket, recvBuf, MAX_DATA_SIZE, 0);

	if (ret == 0)
	{
		TRACE_LOG("Connection closed\n");
		closesocket(mConnectSocket);
		WSACleanup();
	}

	return ret;
}

void NetworkClient::Update()
{
	int dataLength = ReceiveData(mNetworkData);

	if (dataLength <= 0) return;

	Packet packet;
	for (int i = 0; i < dataLength; i += sizeof(Packet))
	{
		packet.Deserialize(&(mNetworkData[i]));
		switch (packet.Type) {
			case SET_CLIENT_ID:
				mID = packet.ClientID;
				break;
			case SPAWN_EXPLORER:
				NetworkRpc::SpawnExistingExplorer(packet.UUID, packet.AsTransform.Position);
				break;
			case DISCONNECT:
				NetworkRpc::DisconnectExplorer(packet.UUID);
				break;
			case SPAWN_SKILL:
				NetworkRpc::SpawnExistingSkill(packet.AsSkill.Type, packet.UUID, packet.AsSkill.Position, packet.AsSkill.Duration);
				break;
			case GRANT_AUTHORITY:
				NetworkRpc::GrantAuthority(packet.UUID);
				break;
			case SYNC_TRANSFORM:
				NetworkRpc::SyncTransform(packet.UUID,
					packet.AsTransform.Position, packet.AsTransform.Rotation);
				break;
			case SYNC_HEALTH:
				NetworkRpc::SyncHealth(packet.UUID, packet.AsFloatArray[0], packet.AsFloatArray[1], packet.AsFloatArray[2]);
				break;
			case SYNC_ANIMATION:
				NetworkRpc::SyncAnimation(packet.UUID, packet.AsAnimation.State, packet.AsAnimation.Command);
				break;
			case INTERACT:
				NetworkRpc::Interact(packet.UUID);
				break;
			case READY:
				NetworkRpc::Ready(packet.ClientID, packet.AsBool);
				break;
			case RESTART:
				NetworkRpc::Restart();
				break;
			default:
				TRACE_LOG("error in packet types\n");
				break;
		}
	}
}


int NetworkClient::SendData(Packet* p) {
	p->Serialize(mPacketData);

	return send(mConnectSocket, mPacketData, sizeof(mPacketData), 0);
}