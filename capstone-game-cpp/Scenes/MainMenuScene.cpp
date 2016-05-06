#include "stdafx.h"
#include "MainMenuScene.h"
#include "Factory.h"
#include "Level01.h"
#include <ScareTacticsApplication.h>

#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"
#include <SceneObjects/Explorer.h>
#include <Components/ExplorerController.h>
#include <Rig3D/Graphics/Interface/IShaderResource.h>

bool MainMenuScene::gLocalClient = false;
bool MainMenuScene::gLocalServer = false;
bool MainMenuScene::gDebugBoth = false;
extern bool gDebugExplorer;

void MainMenuScene::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	mAllocator.SetMemory(mStaticMemory, mStaticMemory + mStaticMemorySize);

	mRenderer->VCreateShaderResource(&mResource, &mAllocator);

	const char* filenames[] = {
		"Assets/UI/splash.png"
	};

	mRenderer->VAddShaderTextures2D(mResource, filenames, 1);
	mRenderer->VAddShaderPointSamplerState(mResource, SAMPLER_STATE_ADDRESS_WRAP);

	// Initialization code here
	strcpy_s(mIPAdress,"localhost");

	mState = BASE_SCENE_STATE_RUNNING;
}

void MainMenuScene::VUpdate(double milliseconds)
{

}

void MainMenuScene::VFixedUpdate(double milliseconds)
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

	mRenderer->VSetInputLayout(mApplication->mVSFwdFullScreenQuad);
	mRenderer->VSetVertexShader(mApplication->mVSFwdFullScreenQuad);
	mRenderer->VSetPixelShader(mApplication->mPSFwdTexture);

	mRenderer->VSetPixelShaderResourceView(mResource, 0, 0);

	mRenderer->GetDeviceContext()->Draw(3, 0);

	RenderIMGUI(&RenderMainMenu);

	mRenderer->VSwapBuffers();
}

void MainMenuScene::RenderMainMenu(BaseScene* s)
{
	MainMenuScene* scene = static_cast<MainMenuScene*>(s);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImGuiAlign_Center;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.56f, 0.30f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.67f, 0.51f, 1.00f, 0.26f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.34f, 0.00f, 0.55f, 0.94f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.27f, 0.40f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.13f, 0.13f, 0.13f, 0.65f);
	
	auto w = scene->mRenderer->GetWindowWidth();
	auto h = scene->mRenderer->GetWindowHeight();

#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(0.439f * w, 0.528f * h), ImGuiSetCond_Always);
	ImGui::SetNextWindowContentWidth(0.15f * w);
#elif
	ImGui::SetNextWindowPos(ImVec2(0.439f * w, 0.55f * h), ImGuiSetCond_Always);
	ImGui::SetNextWindowContentSize(ImVec2(0.15f * w, 0.266f * h));
#endif

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 3));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	ImGui::Begin("Scare Tactics", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	
	ImGui::SetCursorPosX(0.04f * w);
	if (ImGui::Button("Host Match", ImVec2(0.08f * w,0)) || scene->mInput->GetKeyDown(KEYCODE_1) || gLocalServer)
	{
		if (!scene->StartServer()) ImGui::OpenPopup("Error");
	}

	ImGui::Separator();
	ImGui::SetCursorPosX((0.16f * w - ImGui::CalcTextSize("IP Address").x) / 2);
	ImGui::LabelText("##IPLabel", "IP Address");
	ImGui::SetCursorPosX(0.03f * w);
	ImGui::PushItemWidth(0.1f * w);
	ImGui::InputText("##IPInput", scene->mIPAdress, 40);
	ImGui::PopItemWidth();

	ImGui::SetCursorPosX(0.04f * w);
	if (ImGui::Button("Join Match", ImVec2(0.08f * w, 0)) || scene->mInput->GetKeyDown(KEYCODE_2) || gLocalClient)
	{
		if (!scene->StartClient()) ImGui::OpenPopup("Error");
	}

	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(scene->mErrorMsg);
		ImGui::SetCursorPosX(50);
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
	if (ImGui::Button("Server -> Create Explorer", ImVec2(0.1f * w, 0)) || (gDebugBoth))
	{
		scene->StartServer();
		gDebugExplorer = true;
	}

	ImGui::SetCursorPosX(50);
	if (ImGui::Button("Server -> New Client", ImVec2(0.1f * w, 0)) || (gLocalServer && gLocalClient))
	{
		scene->StartServer();
		system("start Bin/Debug/capstone-game-cpp.exe --local-client");
	}

	ImGui::SetCursorPosX(50);
	if (ImGui::Button("Client -> New Server", ImVec2(0.1f * w, 0)))
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
	mResource->~IShaderResource();

	mAllocator.Free();
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
