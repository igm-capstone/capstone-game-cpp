#pragma once

#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <map>

#pragma comment (lib, "Ws2_32.lib")

#include "NetworkData.h"

class NetworkServer
{
	friend class NetworkManager;

private:
	SOCKET mListenSocket;
	SOCKET mClientSocket;
	char mNetworkData[MAX_PACKET_SIZE];
	
	unsigned int mClientID = 0;
	std::map<unsigned int, SOCKET> mClientList;

public:
	NetworkServer(void) {};
	~NetworkServer(void) {};

	void SendToAll(char* buf, int bufSize);
	
protected:
	bool Init();
	void Update();
	void Shutdown();

private:
	void CheckForNewClients();
	void ReceiveFromClients();
	int ReceiveData(unsigned int clientID, char* recvBuf);
};

