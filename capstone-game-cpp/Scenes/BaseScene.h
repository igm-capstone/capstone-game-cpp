#pragma once
#include "stdafx.h"

#include "Network\NetworkManager.h"
#include <SceneObjects/BaseSceneObject.h>
#include <CameraManager.h>
#include <Components/ColliderComponent.h>

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

	CameraManager*				mCameraManager;
	NetworkManager*				mNetworkManager;
	BaseSceneObject*			mMe;

	uint8_t*					mStaticMemory;
	size_t						mStaticMemorySize;

public:
	BaseSceneState				mState;

	// Miscelanious
	BoxCollider					mFloorCollider;

	BaseScene();
	virtual ~BaseScene();

	void SetStaticMemory(void* staticMemory, size_t size);

	// Common methods
	void RenderFPSIndicator();
	void RenderIMGUI(void(*)(BaseScene*) = nullptr);
};

