#include "stdafx.h"
#include "BaseScene.h"
#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Explorer.h>
#include <SceneObjects/Ghost.h>
#include <SceneObjects/Minion.h>

BaseScene::BaseScene() : 
	mStaticMemory(nullptr),
	mStaticMemorySize(0),
	mState(BASE_SCENE_STATE_CONSTRUCTED),
	mMe(nullptr)
{
	mEngine = &Singleton<Engine>::SharedInstance();

	mRenderer = mEngine->GetRenderer();
	mDevice = mRenderer->GetDevice();
	mDeviceContext = mRenderer->GetDeviceContext();
	mRenderer->SetDelegate(this);

	mInput = mEngine->GetInput();

	mNetworkManager = &Singleton<NetworkManager>::SharedInstance();

	if (mNetworkManager->mMode == NetworkManager::Mode::CLIENT) {
		Packet p(PacketTypes::INIT_CONNECTION);
		mNetworkManager->mClient.SendData(&p);
	} 
	else if (mNetworkManager->mMode == NetworkManager::Mode::SERVER) {
		mMe = Factory<Ghost>::Create();
	}

}

BaseScene::~BaseScene()
{
	// MAKE SURE TO FREE MEMORY HERE OR BEFORE THIS IS CALLED!
}

void BaseScene::SetStaticMemory(void* staticMemory, size_t size)
{
	mStaticMemory = reinterpret_cast<uint8_t*>(staticMemory);
	mStaticMemorySize = size;
}

void BaseScene::RenderFPSIndicator()
{
	ImGui::SetNextWindowPos(ImVec2(mRenderer->GetWindowWidth() - 100.0f, 10.0f), ImGuiSetCond_Always);
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("%6.1f FPS ", ImGui::GetIO().Framerate);
	ImGui::End();
}

#pragma region Network Callbacks

void BaseScene::CmdSpawnNewMinion(vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER);
	
	auto m = Factory<Minion>::Create();
	m->Spawn(pos, MyUUID::GenUUID());
	
	// set auhtority for the server
	m->mNetworkID->mHasAuthority = true;
	m->mNetworkID->OnNetAuthorityChange(true);

	Packet p(PacketTypes::SPAWN_MINION);
	p.UUID = m->mNetworkID->mUUID;
	p.Position = pos;
	mNetworkManager->mServer.SendToAll(&p);
}

void BaseScene::RpcSpawnExistingMinion(int UUID, vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::CLIENT);
	
	auto m = Factory<Minion>::Create();
	m->Spawn(pos, UUID);
}

void BaseScene::CmdSpawnNewExplorer(int clientID) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER);
	// Get a spawn point
	// FIMXE: logic to select spawn point
	SpawnPoint& sp = *(Factory<SpawnPoint>().begin());

	auto e = Factory<Explorer>::Create();
	e->Spawn(sp.mTransform->GetPosition(), MyUUID::GenUUID());

	Packet p(PacketTypes::SPAWN_EXPLORER);
	p.UUID = e->mNetworkID->mUUID;
	p.Position = sp.mTransform->GetPosition();
	mNetworkManager->mServer.SendToAll(&p);

	Packet p2(PacketTypes::GRANT_AUTHORITY);
	p2.UUID = e->mNetworkID->mUUID;
	mNetworkManager->mServer.Send(clientID, &p2);

	//Send existing elements
	for each(Explorer &exp in Factory<Explorer>()) {
		if (&exp == e) continue;
		p.UUID = exp.mNetworkID->mUUID;
		p.Position = exp.mTransform->GetPosition();
		mNetworkManager->mServer.Send(clientID, &p);
	}
}

void BaseScene::RpcSpawnExistingExplorer(int UUID, vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::CLIENT);
	auto e = Factory<Explorer>::Create();
	e->Spawn(pos, UUID);
	if (!mMe) mMe = e;
}


void BaseScene::GrantAuthority(int UUID) {
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == UUID) {
			netID.mHasAuthority = true;
			netID.OnNetAuthorityChange(true);
		}
	}
}

void BaseScene::SyncTransform(int UUID, vec3f pos)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnNetSyncTransform(pos);
		}
	}
}
#pragma endregion
