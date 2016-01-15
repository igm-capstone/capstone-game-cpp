#pragma once

#include "Network\NetworkManager.h"

class MainMenuScene : public Rig3D::IScene, public virtual Rig3D::IRendererDelegate
{
	Rig3D::Renderer*		mRenderer;
	ID3D11DeviceContext*	mDeviceContext;

	NetworkManager*			mNetworkManager;

public:
	MainMenuScene() {};
	~MainMenuScene() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;
};
