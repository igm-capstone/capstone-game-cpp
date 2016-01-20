#pragma once
#include "stdafx.h"

#include "Network\NetworkManager.h"

using namespace Rig3D;

typedef enum BaseSceneState
{
	BASE_SCENE_STATE_INITIAL,	// Upon Construction
	BASE_SCENE_STATE_LOADING,	// Start of VInitialize()
	BASE_SCENE_STATE_RUNNING,	// Completion of VInitialize()
	BASE_SCENE_STATE_UNLOADING	// Start of VShutdown()
};

class BaseScene : public IScene, public virtual IRendererDelegate
{
protected:
	Engine*					mEngine;
	Input*					mInput;
	Renderer*				mRenderer;
	IApplication*			mApplication;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;

	NetworkManager*			mNetworkManager;

	uint8_t*				mStaticMemory;
	size_t					mStaticMemorySize;

public:
	BaseSceneState			mState;

	BaseScene();
	virtual ~BaseScene();

	void SetStaticMemory(void* staticMemory, size_t size);
};

