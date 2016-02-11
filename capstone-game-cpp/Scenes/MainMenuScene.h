#pragma once
#include "Scenes\BaseScene.h"

class MainMenuScene : public BaseScene
{
	char* mErrorMsg;
	char mIPAdress[40];

public:
	MainMenuScene() {};
	~MainMenuScene() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;
	
	static void RenderMainMenu(BaseScene* s);
};
