#include "stdafx.h"
#include "NetworkManager.h"

NetworkManager::NetworkManager()
{
	
}

NetworkManager::~NetworkManager()
{
}

bool NetworkManager::StartServer()
{
	if (mMode == IDLE) {
		if (mServer.Init()) {
			mMode = SERVER;
			return true;
		}
	}
	return false;
}

bool NetworkManager::StopServer()
{
	if (mMode == SERVER) {
		mServer.Shutdown();
		mMode = IDLE;
		return true;
	}
	return false;
}

bool NetworkManager::StartClient()
{
	if (mMode == IDLE) {
		if (mClient.Init()) {
			mMode = CLIENT;
			return true;
		}
	}
	return false;
}

bool NetworkManager::StopClient()
{
	if (mMode == CLIENT) {
		mClient.Shutdown();
		mMode = IDLE;
		return true;
	}
	return false;
}

void NetworkManager::Update()
{
	if (mMode == SERVER) {
		mServer.Update();
	} 
	else if (mMode == CLIENT) {
		mClient.Update();
	}
}

int MyUUID::currentID = 0;