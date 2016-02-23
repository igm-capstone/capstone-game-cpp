#include "stdafx.h"
#include "MainMenuScene.h"
#include "Factory.h"
#include "Level01.h"
#include <ScareTacticsApplication.h>

#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"
#include <SceneObjects/Explorer.h>
#include <Components/ExplorerController.h>

bool MainMenuScene::gLocalClient = false;
bool MainMenuScene::gLocalServer = false;
bool MainMenuScene::gDebugBoth = false;

void MainMenuScene::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	// Initialization code here
	strcpy_s(mIPAdress,"localhost");

	mState = BASE_SCENE_STATE_RUNNING;
}

void MainMenuScene::VUpdate(double milliseconds)
{

}

void MainMenuScene::VRender()
{
	float bg[4] = { .25f, .25f, .25f, 0.5f };

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	
	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr/*mRenderer->GetDepthStencilView()*/);
	mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), bg);
	mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	RenderIMGUI(&RenderMainMenu);

	mRenderer->VSwapBuffers();
}

void MainMenuScene::RenderMainMenu(BaseScene* s)
{
	MainMenuScene* scene = static_cast<MainMenuScene*>(s);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImGuiAlign_Center;
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.67f, 0.51f, 1.00f, 0.26f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.34f, 0.00f, 0.55f, 0.94f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.27f, 0.40f, 0.44f, 1.00f);

	ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
	ImGui::SetNextWindowContentWidth(300);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 3));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	ImGui::Begin("Scare Tactics", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	
	ImGui::SetCursorPosX(70);
	if (ImGui::Button("Host Match", ImVec2(160,0)) || scene->mInput->GetKeyDown(KEYCODE_1) || gLocalServer)
	{
		if (!scene->StartServer()) ImGui::OpenPopup("Error");
	}

	ImGui::Separator();
	ImGui::SetCursorPosX((300 - ImGui::CalcTextSize("IP Address").x) / 2);
	ImGui::LabelText("##IPLabel", "IP Address");
	ImGui::SetCursorPosX(50);
	ImGui::PushItemWidth(200);
	ImGui::InputText("##IPInput", scene->mIPAdress, 40);
	ImGui::PopItemWidth();

	ImGui::SetCursorPosX(70);
	if (ImGui::Button("Join Match", ImVec2(160, 0)) || scene->mInput->GetKeyDown(KEYCODE_2) || gLocalClient)
	{
		if (!scene->StartClient()) ImGui::OpenPopup("Error");
	}

	if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(scene->mErrorMsg);
		ImGui::SetCursorPosX(60);
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

#ifdef _DEBUG
	ImGui::Separator();
	ImGui::PopFont();
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	ImGui::SetCursorPosX(50);
	if (ImGui::Button("Server -> Create Explorer", ImVec2(200, 0)) || (gDebugBoth))
	{
		scene->StartServer();
		auto e = Factory<Explorer>::Create();
		e->mTransform->SetPosition(0.0f, 0.0f, 7.5f);	// Centering in level based on Unity. Can be fixed when we get Z - Extents in JSON.
		e->mController->mIsActive = true;
	}

	ImGui::SetCursorPosX(50);
	if (ImGui::Button("Server -> New Client", ImVec2(200, 0)) || (gLocalServer && gLocalClient))
	{
		scene->StartServer();
		system("start Bin/Debug/capstone-game-cpp.exe --local-client");
	}

	ImGui::SetCursorPosX(50);
	if (ImGui::Button("Client -> New Server", ImVec2(200, 0)))
	{
		system("start Bin/Debug/capstone-game-cpp.exe --local-server");
		Sleep(3);
		scene->StartClient();
	}
#endif

	ImGui::End();
	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void MainMenuScene::VShutdown()
{
	mState = BASE_SCENE_STATE_SHUTDOWN;

	// Shutdown code here
}

void MainMenuScene::VOnResize()
{
}

bool MainMenuScene::StartServer()
{
	if (mNetworkManager->StartServer()) {
		Application::SharedInstance().LoadScene<Level01>();
		return true;
	} 
	mErrorMsg = "Failed to host a server";
	return false;
}

bool MainMenuScene::StartClient()
{
	mNetworkManager->mClient.mIPAddress = mIPAdress;
	if (mNetworkManager->StartClient()) {
		Application::SharedInstance().LoadScene<Level01>();
		return true;
	}
	mErrorMsg = "Failed to connect.";
	return false;
}
