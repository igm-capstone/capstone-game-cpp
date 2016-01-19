﻿#include "stdafx.h"
#include "MainMenuScene.h"
#include "Level00.h"

void MainMenuScene::VInitialize()
{

}

void MainMenuScene::VUpdate(double milliseconds)
{
	if (mInput->GetKeyDown(KEYCODE_1))
	{
		mNetworkManager->StartServer();
		mEngine->GetApplication()->LoadScene<Level00>();
	}
	if (mInput->GetKeyDown(KEYCODE_2))
	{
		mNetworkManager->StartClient();
		mEngine->GetApplication()->LoadScene<Level00>();
	}
}

void MainMenuScene::VRender()
{
	float bg[4] = { 1.0f, 0.2f, 0.2f, 0.5f };

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	
	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
	mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), bg);
	mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mRenderer->VSwapBuffers();
}

void MainMenuScene::VShutdown()
{
}

void MainMenuScene::VOnResize()
{
}