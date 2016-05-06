#pragma once
#include "Scenes\BaseScene.h"

class MainMenuScene : public BaseScene
{
	char* mErrorMsg;
	char mIPAdress[40];

public:
	static bool gLocalClient;
	static bool gLocalServer;
	static bool gDebugBoth;

	MainMenuScene() {};
	~MainMenuScene() override {}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VFixedUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;
	
	bool StartServer();
	bool StartClient();

	static void RenderMainMenu(BaseScene* s);

	LinearAllocator		mAllocator;

	IShaderResource* mResource;
};
