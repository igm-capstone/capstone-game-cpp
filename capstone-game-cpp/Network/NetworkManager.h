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
		CLIENT
	};

	Mode mMode;

	NetworkServer mServer;
	NetworkClient mClient;

	int ID() { return (mMode == CLIENT ? mClient.mID : (mMode == SERVER ? 0 : -1)); }

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
	static void SpawnNewSkill(SkillPacketTypes type, vec3f pos, float duration, int targetUUID = -1);
};

class NetworkRpc {
	static NetworkManager* mNetworkManager;
public:
	static void SpawnExistingExplorer(int UUID, vec3f pos);
	static void DisconnectExplorer(int UUID);
	static void SpawnExistingSkill(SkillPacketTypes type, int UUID, vec3f pos, float duration, int targetUUID = -1);

	static void GrantAuthority(int UUID);
	static void SyncTransform(int UUID, vec3f pos, quatf rot);
	static void SyncHealth(int UUID, float val, float max, float dir);
	static void SyncAnimation(int UUID, byte state, byte command);
	static void Interact(int UUID);
	static void Ready(unsigned clientID, bool isReady);
};


class MyUUID {
	static int currentID;
public:
	static int GenUUID() {
		return ++currentID;
	}
};

