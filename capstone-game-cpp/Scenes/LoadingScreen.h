#pragma once
#include "Scenes/common.h"

using namespace Rig3D;

class LoadingScreen : public IScene, public virtual IRendererDelegate
{
public:
	LoadingScreen() {};
	~LoadingScreen() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;

private:
	float mBgColor[4] = { 0.2f, 0.2f, 0.2f, 0.5f };

	DX3D11Renderer*			mRenderer;
	ID3D11DeviceContext*	mDeviceContext;
};