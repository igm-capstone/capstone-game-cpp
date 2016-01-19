#include "stdafx.h"
#include "NetworkClient.h"

bool NetworkClient::Init(void)
{
	WSADATA wsaData;

	mConnectSocket = INVALID_SOCKET;

	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) {
		printf("WSAStartup failed: %d\n", ret);
		return false;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	ret = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (ret != 0) {
		printf("getaddrinfo failed: %d\n", ret);
		WSACleanup();
		return false;
	}
	
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		mConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (mConnectSocket == INVALID_SOCKET) {
			printf("socket failed: %ld\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		ret = connect(mConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (ret == SOCKET_ERROR)
		{
			closesocket(mConnectSocket);
			mConnectSocket = INVALID_SOCKET;
			printf("The server is down... did not connect");
		}
	}

	freeaddrinfo(result);

	if (mConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return false;
	}

	u_long iMode = 1;
	ret = ioctlsocket(mConnectSocket, FIONBIO, &iMode); //non-blocking mode
	if (ret == SOCKET_ERROR) {
		printf("ioctlsocket failed: %d\n", WSAGetLastError());
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
	int ret = recv(mConnectSocket, recvBuf, MAX_PACKET_SIZE, 0);

	if (ret == 0)
	{
		printf("Connection closed\n");
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
			case CHNAGE_COLOR:
				printf("client received action event packet from server\n");
				break;
			default:
				printf("error in packet types\n");
				break;
		}
	}
}