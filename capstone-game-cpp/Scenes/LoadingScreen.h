#pragma once

class LoadingScreen : public Rig3D::IScene, public virtual Rig3D::IRendererDelegate
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
	Rig3D::Renderer*		mRenderer;
	ID3D11DeviceContext*	mDeviceContext;
};