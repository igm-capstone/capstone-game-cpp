#include "stdafx.h"
#include "BaseScene.h"
#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"
#include <SceneObjects/SpawnPoint.h>

BaseScene::BaseScene() : 
	mStaticMemory(nullptr),
	mStaticMemorySize(0),
	mState(BASE_SCENE_STATE_CONSTRUCTED)
{
	mEngine = &Singleton<Engine>::SharedInstance();

	mRenderer = mEngine->GetRenderer();
	mDevice = mRenderer->GetDevice();
	mDeviceContext = mRenderer->GetDeviceContext();
	mRenderer->SetDelegate(this);

	mInput = mEngine->GetInput();

	mNetworkManager = &Singleton<NetworkManager>::SharedInstance();
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
	ImGui::SetNextWindowPos(ImVec2(mRenderer->GetWindowWidth() - 100, 10), ImGuiSetCond_Always);
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("%6.1f FPS ", ImGui::GetIO().Framerate);
	ImGui::End();
}

#pragma region Network Callbacks
void BaseScene::CmdSpawnNewExplorer(int clientID) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER, "Cmd being called on non-server");
	// Get a spawn point
	// FIMXE: logic to select spawn point
	SpawnPoint& sp = *(Factory<SpawnPoint>().begin());

	auto e = Factory<Explorer>::Create();
	e->Spawn(sp.mTransform->GetPosition(), MyUUID::GenUUID());

	Packet p(PacketTypes::SPAWN_EXPLORER);
	p.UUID = e->mNetworkID->mUUID;
	mNetworkManager->mServer.SendToAll(&p);

	Packet p2(PacketTypes::GRANT_AUTHORITY);
	p2.UUID = e->mNetworkID->mUUID;
	mNetworkManager->mServer.Send(clientID, &p2);
}

void BaseScene::RpcSpawnExistingExplorer(int UUID, vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER, "Rpc being called on non-client");
	auto e = Factory<Explorer>::Create();
	e->Spawn(pos, UUID);
}


void BaseScene::GrantAuthority(int UUID) {
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == UUID) {
			netID.mHasAuthority = true;
			((Explorer*)netID.mSceneObject)->mController->mIsActive = true;
		}
	}
}

void BaseScene::SyncTransform(int UUID, vec3f pos)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.mSceneObject->MoveTo(pos);
	}
}
#pragma endregion
