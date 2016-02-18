#include "stdafx.h"
#include "NetworkManager.h"
#include <Factory.h>
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Explorer.h>
#include <SceneObjects/Minion.h>

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
NetworkManager* NetworkCmd::mNetworkManager = &Rig3D::Singleton<NetworkManager>::SharedInstance();
NetworkManager* NetworkRpc::mNetworkManager = &Rig3D::Singleton<NetworkManager>::SharedInstance();

void NetworkCmd::SpawnNewExplorer(int clientID) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER);
	// Get a spawn point
	// FIMXE: logic to select spawn point
	SpawnPoint& sp = *(Factory<SpawnPoint>().begin());

	auto e = Factory<Explorer>::Create();
	e->Spawn(sp.mTransform->GetPosition(), MyUUID::GenUUID());

	Packet p(PacketTypes::SPAWN_EXPLORER);
	p.UUID = e->mNetworkID->mUUID;
	p.AsTransform.Position = sp.mTransform->GetPosition();
	mNetworkManager->mServer.SendToAll(&p);

	Packet p2(PacketTypes::GRANT_AUTHORITY);
	p2.UUID = e->mNetworkID->mUUID;
	mNetworkManager->mServer.Send(clientID, &p2);

	//Send existing elements
	for each(Explorer &exp in Factory<Explorer>()) {
		if (&exp == e) continue;
		p.UUID = exp.mNetworkID->mUUID;
		p.AsTransform.Position = exp.mTransform->GetPosition();
		mNetworkManager->mServer.Send(clientID, &p);
	}
}

void NetworkRpc::SpawnExistingExplorer(int UUID, vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::CLIENT);
	auto e = Factory<Explorer>::Create();
	e->Spawn(pos, UUID);
}

void NetworkCmd::SpawnNewMinion(vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER);

	auto m = Factory<Minion>::Create();
	m->Spawn(pos, MyUUID::GenUUID());

	// set auhtority for the server
	m->mNetworkID->mHasAuthority = true;
	m->mNetworkID->OnNetAuthorityChange(true);

	Packet p(PacketTypes::SPAWN_MINION);
	p.UUID = m->mNetworkID->mUUID;
	p.AsTransform.Position = pos;
	mNetworkManager->mServer.SendToAll(&p);
}

void NetworkRpc::SpawnExistingMinion(int UUID, vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::CLIENT);

	auto m = Factory<Minion>::Create();
	m->Spawn(pos, UUID);
}

void NetworkRpc::GrantAuthority(int UUID) {
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == UUID) {
			netID.mHasAuthority = true;
			netID.OnNetAuthorityChange(true);
		}
	}
}

void NetworkRpc::SyncTransform(int UUID, vec3f pos, quatf rot)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnNetSyncTransform(pos, rot);
		}
	}
}

void NetworkRpc::SyncHealth(int UUID, float val)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnNetHealthChange(val);
		}
	}
}
