#include "stdafx.h"
#include "BaseScene.h"
#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Explorer.h>
#include <SceneObjects/Ghost.h>
#include <SceneObjects/Minion.h>
#include <Rig3D/Graphics/DirectX11/DX11IMGUI.h>
#include <Console.h>

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

	mCameraManager = &Singleton<CameraManager>::SharedInstance();
	mCameraManager->Initialize(mRenderer->GetAspectRatio());
	
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

/* Renders IMGUI. ideally, it is the last call in the render loop.
 * Optionally takes a void(*)(BaseScene*) function that draw custom IMGUI on top of the default FPS and Console. */
void BaseScene::RenderIMGUI(void(*IMGUIDrawFunc)(BaseScene*))
{
	mRenderer->VSetContextTarget();
	DX11IMGUI::NewFrame();
	RenderFPSIndicator();
	Console::Draw();
	if (IMGUIDrawFunc) IMGUIDrawFunc(this);
	ImGui::Render();
}