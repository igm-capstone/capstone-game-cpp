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
	char mNetworkData[MAX_DATA_SIZE];
	char mPacketData[sizeof(Packet)];

	SOCKET mClientList[4] = { 0 };
	int mClientCount = 0;
	void RemoveClient(const unsigned clientID);

public:
	NetworkServer(void) {};
	~NetworkServer(void) {};

	void SendToAll(Packet* p);
	void Send(int id, Packet* p);
	void Retransmit(int originalSender, Packet* p);
	
protected:
	bool Init();
	void Update();
	void Shutdown();

private:
	void CheckForNewClients();
	void ReceiveFromClients();
};

