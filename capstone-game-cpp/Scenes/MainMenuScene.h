﻿#pragma once
#include "Scenes\BaseScene.h"

class MainMenuScene : public BaseScene
{
	bool failedClient = false;

public:
	MainMenuScene() {};
	~MainMenuScene() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void RenderUI();
	void VShutdown() override;
	void VOnResize() override;
};
