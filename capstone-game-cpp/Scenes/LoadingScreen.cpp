#include "stdafx.h"
#include "LoadingScreen.h"

using namespace Rig3D;

void LoadingScreen::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	// Initialization code here

	mState = BASE_SCENE_STATE_RUNNING;
}

void LoadingScreen::VUpdate(double milliseconds)
{
}

void LoadingScreen::VRender()
{
	float bg[4] = { 0.2f, 0.2f, 0.2f, 0.5f };

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);

	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
	mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), bg);
	mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mRenderer->VSwapBuffers();
}

void LoadingScreen::VShutdown()
{
	mState = BASE_SCENE_STATE_SHUTDOWN;

	// Shutdown code here
}

void LoadingScreen::VOnResize()
{
}