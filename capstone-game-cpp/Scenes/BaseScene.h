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

public:
	BaseScene();
	~BaseScene();
};

