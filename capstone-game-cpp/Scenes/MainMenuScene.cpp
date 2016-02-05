#include "stdafx.h"
#include "MainMenuScene.h"
#include "Level00.h"
#include "Level01.h"
#include <ScareTacticsApplication.h>
#include <Console.h>

#include "Rig3D/Graphics/DirectX11/DX11IMGUI.h"
#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"

void MainMenuScene::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	// Initialization code here
	strcpy(mIPAdress,"localhost");

	mState = BASE_SCENE_STATE_RUNNING;
}

void MainMenuScene::VUpdate(double milliseconds)
{
	if (mInput->GetKeyDown(KEYCODE_1))
	{
		mNetworkManager->StartServer();
		Application::SharedInstance().LoadScene<Level01>();
	}
	if (mInput->GetKeyDown(KEYCODE_2))
	{
		if (mNetworkManager->StartClient())
			Application::SharedInstance().LoadScene<Level01>();
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
	Console::Draw();
	
	ImGui::Render();

	mRenderer->VSwapBuffers();
}

void MainMenuScene::RenderUI()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImGuiAlign_Center;
	ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
	ImGui::SetNextWindowContentWidth(300);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	ImGui::Begin("Scare Tactics", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	
	if (ImGui::Button("Host Match"))
	{
		if (mNetworkManager->StartServer())
			Application::SharedInstance().LoadScene<Level01>();
		else {
			ImGui::OpenPopup("Error");
			mErrorMsg = "Failed to host a server";
		}
	}

	ImGui::Separator();

	ImGui::PushItemWidth(175);
	ImGui::LabelText("##IPLabel", "IP Address");
	ImGui::InputText("##IPInput", mIPAdress, 40);
	ImGui::PopItemWidth();

	if (ImGui::Button("Join Match"))
	{
		mNetworkManager->mClient.mIPAddress = mIPAdress;
		if (mNetworkManager->StartClient())
			Application::SharedInstance().LoadScene<Level01>();
		else {
			ImGui::OpenPopup("Error");
			mErrorMsg = "Failed to connect.";
		}
	}



	ImGui::Separator();

	if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(mErrorMsg);

		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	
#ifdef _DEBUG
	if (ImGui::Button("Start Debug"))
	{
		mNetworkManager->StartServer();
		Application::SharedInstance().LoadScene<Level01>();
		auto e = Factory<Explorer>::Create();
		Explorer::OnNetAuthorityChange(e, true);
	}
#endif
	ImGui::End();
	ImGui::PopFont();
}

void MainMenuScene::VShutdown()
{
	mState = BASE_SCENE_STATE_SHUTDOWN;

	// Shutdown code here
}

void MainMenuScene::VOnResize()
{
}