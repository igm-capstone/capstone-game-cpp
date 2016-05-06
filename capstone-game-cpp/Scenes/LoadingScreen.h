#pragma once
#include "Scenes\BaseScene.h"

class LoadingScreen : public BaseScene
{
public:
	LoadingScreen() {};
	~LoadingScreen() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VFixedUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;
};