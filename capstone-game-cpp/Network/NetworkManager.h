#pragma once

#include <winsock2.h>
#include <Windows.h>

#include "NetworkServer.h"
#include "NetworkClient.h"


class NetworkManager
{
public:
	enum Mode {
		IDLE,
		SERVER,
		CLIENT,
		BOTH
	};

	Mode mMode;

	NetworkServer mServer;
	NetworkClient mClient;

	bool StartServer();
	bool StartClient();
	bool StopServer();
	bool StopClient();

	void Update();

	NetworkManager();
	~NetworkManager();
};




class MyUUID {
	static int currentID;
public:
	static int GenUUID() {
		return ++currentID;
	}
};

