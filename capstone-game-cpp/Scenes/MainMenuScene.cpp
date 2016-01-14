#include "stdafx.h"
#include "MainMenuScene.h"
#include <Rig3D/Singleton.h>
#include "Level00.h"

using namespace Rig3D;

void MainMenuScene::VInitialize()
{
	Engine& engine = Singleton<Engine>::SharedInstance();
	mRenderer = engine.GetRenderer();
	mDeviceContext = mRenderer->GetDeviceContext();

	mRenderer->SetDelegate(this);
}

void MainMenuScene::VUpdate(double milliseconds)
{
	Engine& engine = Singleton<Engine>::SharedInstance();
	auto input = engine.GetInput();

	if (input->GetKeyDown(KEYCODE_1))
	{
		engine.GetApplication()->LoadScene<Level00>();
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