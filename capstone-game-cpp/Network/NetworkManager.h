#pragma once

#include <winsock2.h>
#include <Windows.h>

#include "NetworkServer.h"
#include "NetworkClient.h"


class NetworkManager
{
	enum Mode {
		IDLE,
		SERVER,
		CLIENT,
		BOTH
	};

public:
	NetworkManager();
	~NetworkManager();

	static unsigned int mClientID;
	Mode mMode;

	NetworkServer mServer;
	NetworkClient mClient;

	bool StartServer();
	bool StartClient();
	bool StopServer();
	bool StopClient();

	void Update();
};

