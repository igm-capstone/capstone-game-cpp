#include "stdafx.h"
#include "MainMenuScene.h"
#include "Level00.h"
#include "Level01.h"
#include <ScareTacticsApplication.h>

#include "Rig3D/Graphics/DirectX11/DX11IMGUI.h"
#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"

void MainMenuScene::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	// Initialization code here

	mState = BASE_SCENE_STATE_RUNNING;
}

void MainMenuScene::VUpdate(double milliseconds)
{
	if (mInput->GetKeyDown(KEYCODE_1))
	{
		mNetworkManager->StartServer();
		Application::SharedInstance().LoadScene<Level00>();
	}
	if (mInput->GetKeyDown(KEYCODE_2))
	{
		if (mNetworkManager->StartClient())
			Application::SharedInstance().LoadScene<Level00>();
	}
	if (mInput->GetKeyDown(KEYCODE_3))
	{
		Application::SharedInstance().LoadScene<Level01>();
	}
}

void MainMenuScene::VRender()
{
	float bg[4] = { .25f, .25f, .25f, 0.5f };

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	
	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr/*mRenderer->GetDepthStencilView()*/);
	mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), bg);
	mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DX11IMGUI::NewFrame();
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr);
	RenderUI();
	RenderFPSIndicator();
	ImGui::Render();

	mRenderer->VSwapBuffers();
}

void MainMenuScene::RenderUI()
{
	ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
	ImGui::Begin("Scare Tactics", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::Button("Start Server"))
	{
		mNetworkManager->StartServer();
		Application::SharedInstance().LoadScene<Level01>();
	}
	if (ImGui::Button("Start Client"))
	{
		if (mNetworkManager->StartClient())
			Application::SharedInstance().LoadScene<Level00>();
		else
			failedClient = true;
	}
	if (failedClient) 
		ImGui::Text("Failed to start client");
	ImGui::End();
}

void MainMenuScene::VShutdown()
{
	mState = BASE_SCENE_STATE_SHUTDOWN;

	// Shutdown code here
}

void MainMenuScene::VOnResize()
{
}