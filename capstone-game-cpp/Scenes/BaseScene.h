#pragma once
#include "stdafx.h"

#include "Network\NetworkManager.h"

using namespace Rig3D;

class BaseScene : public IScene, public virtual IRendererDelegate
{
protected:
	Engine*					mEngine;
	Input*					mInput;
	Renderer*				mRenderer;
	Application*			mApplication;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;

	NetworkManager*			mNetworkManager;

	uint8_t*					mStaticMemory;
	size_t					mStaticMemorySize;

public:
	BaseScene();
	~BaseScene();

	void SetStaticMemory(void* staticMemory, size_t size);
};

