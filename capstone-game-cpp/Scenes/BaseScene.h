#pragma once
#include "stdafx.h"

#include "Network\NetworkManager.h"

using namespace Rig3D;

enum BaseSceneState
{
	BASE_SCENE_STATE_CONSTRUCTED,	// Upon Construction
	BASE_SCENE_STATE_INITIALIZING,	// Start of VInitialize()
	BASE_SCENE_STATE_RUNNING,		// Completion of VInitialize()
	BASE_SCENE_STATE_SHUTDOWN		// Start of VShutdown()
};

class BaseScene : public IScene, public virtual IRendererDelegate
{
	friend class ScareTacticsApplication;

protected:
	Engine*						mEngine;
	Input*						mInput;
	Renderer*					mRenderer;
	ScareTacticsApplication*	mApplication;
	ID3D11Device*				mDevice;
	ID3D11DeviceContext*		mDeviceContext;

	NetworkManager*				mNetworkManager;

	uint8_t*					mStaticMemory;
	size_t						mStaticMemorySize;

public:
	BaseSceneState				mState;

	BaseScene();
	virtual ~BaseScene();

	void SetStaticMemory(void* staticMemory, size_t size);

	// Common methods
	void RenderFPSIndicator();
};

