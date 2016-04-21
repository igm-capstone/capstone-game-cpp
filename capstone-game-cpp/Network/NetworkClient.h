#pragma once

#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdio.h> 

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "NetworkData.h"

class NetworkClient
{
	friend class NetworkManager;

private:
	SOCKET mConnectSocket;
	char mNetworkData[MAX_DATA_SIZE];
	char mPacketData[sizeof(Packet)];

public:
	NetworkClient(void) {};
	~NetworkClient(void) {};

	int mID;
	PCSTR mIPAddress;
	int SendData(Packet* p);

protected:
	bool Init();
	void Update();
	void Shutdown();

private:
	int ReceiveData(char* recvBuf);
};

