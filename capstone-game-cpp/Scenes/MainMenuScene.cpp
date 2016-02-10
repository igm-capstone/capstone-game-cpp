﻿#include "stdafx.h"
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
	if (ImGui::Button("Host Match", ImVec2(160,0)))
	{
		if (mNetworkManager->StartServer())
			Application::SharedInstance().LoadScene<Level01>();
		else {
			ImGui::OpenPopup("Error");
			mErrorMsg = "Failed to host a server";
		}
	}

	ImGui::Separator();
	ImGui::SetCursorPosX((300 - ImGui::CalcTextSize("IP Address").x) / 2);
	ImGui::LabelText("##IPLabel", "IP Address");
	ImGui::SetCursorPosX(50);
	ImGui::PushItemWidth(200);
	ImGui::InputText("##IPInput", mIPAdress, 40);
	ImGui::PopItemWidth();

	ImGui::SetCursorPosX(70);
	if (ImGui::Button("Join Match", ImVec2(160, 0)))
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
		ImGui::SetCursorPosX(60);
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	ImGui::SetCursorPosX(70);
#ifdef _DEBUG
	if (ImGui::Button("Start Debug", ImVec2(160, 0)))
	{
		mNetworkManager->StartServer();
		Application::SharedInstance().LoadScene<Level01>();
		auto e = Factory<Explorer>::Create();
		Explorer::OnNetAuthorityChange(e, true);
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