#include "LoadingScreen.h"
#include "Rig3D/Application.h"
#include "Level00.h"

void LoadingScreen::VInitialize()
{
	mRenderer = &DX3D11Renderer::SharedInstance();
	mDeviceContext = mRenderer->GetDeviceContext();

	mRenderer->SetDelegate(this);
}

void LoadingScreen::VUpdate(double milliseconds)
{
	auto input = &Input::SharedInstance();

	if (input->GetKeyDown(KEYCODE_1))
	{
		Application::SharedInstance().LoadScene<Level00>();
	}
}

void LoadingScreen::VRender()
{
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
	mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), mBgColor);
	mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mRenderer->VSwapBuffers();
}

void LoadingScreen::VShutdown()
{

}

void LoadingScreen::VOnResize()
{
}