#pragma once
#include "Scenes\BaseScene.h"

class MainMenuScene : public BaseScene
{
public:
	MainMenuScene() {};
	~MainMenuScene() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;
};
