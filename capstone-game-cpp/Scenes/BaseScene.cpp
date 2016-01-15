#include "stdafx.h"
#include "BaseScene.h"


BaseScene::BaseScene()
{
	mEngine = &Singleton<Engine>::SharedInstance();

	mRenderer = mEngine->GetRenderer();
	mDevice = mRenderer->GetDevice();
	mDeviceContext = mRenderer->GetDeviceContext();
	mRenderer->SetDelegate(this);

	mApplication = mEngine->GetApplication();

	mInput = mEngine->GetInput();

	mNetworkManager = &Singleton<NetworkManager>::SharedInstance();
}


BaseScene::~BaseScene()
{
}
