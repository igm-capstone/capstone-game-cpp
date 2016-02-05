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

class NetworkCmd
{
	static NetworkManager* mNetworkManager;
public:
	static void SpawnNewExplorer(int clientID);
	static void SpawnNewMinion(vec3f pos);
};

class NetworkRpc {
	static NetworkManager* mNetworkManager;
public:
	static void SpawnExistingExplorer(int UUID, vec3f pos);
	static void SpawnExistingMinion(int UUID, vec3f pos);

	static void GrantAuthority(int UUID);
	static void SyncTransform(int UUID, vec3f pos);
};


class MyUUID {
	static int currentID;
public:
	static int GenUUID() {
		return ++currentID;
	}
};

