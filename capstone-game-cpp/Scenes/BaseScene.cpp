#include "stdafx.h"
#include "BaseScene.h"

BaseScene::BaseScene() : 
	mStaticMemory(nullptr),
	mStaticMemorySize(0),
	mState(BASE_SCENE_STATE_CONSTRUCTED)
{
	mEngine = &Singleton<Engine>::SharedInstance();

	mRenderer = mEngine->GetRenderer();
	mDevice = mRenderer->GetDevice();
	mDeviceContext = mRenderer->GetDeviceContext();
	mRenderer->SetDelegate(this);

	mInput = mEngine->GetInput();

	mNetworkManager = &Singleton<NetworkManager>::SharedInstance();
}

BaseScene::~BaseScene()
{
	// MAKE SURE TO FREE MEMORY HERE OR BEFORE THIS IS CALLED!
}

void BaseScene::SetStaticMemory(void* staticMemory, size_t size)
{
	mStaticMemory = reinterpret_cast<uint8_t*>(staticMemory);
	mStaticMemorySize = size;
}