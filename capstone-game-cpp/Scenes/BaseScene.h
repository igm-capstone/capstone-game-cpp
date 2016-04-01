#pragma once
#include "stdafx.h"

#include "Network\NetworkManager.h"
#include <CameraManager.h>
#include <CollisionManager.h>
#include <AIManager.h>
#include <Resource.h>

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

	// Managers
	CameraManager*				mCameraManager;
	NetworkManager*				mNetworkManager;
	CollisionManager*			mCollisionManager;
	AIManager*					mAIManager;
	ModelManager*				mModelManager;

	uint8_t*					mStaticMemory;
	size_t						mStaticMemorySize;

public:
	BaseSceneState				mState;

	// Miscelanious
	bool						mDebugGrid;
	bool						mDebugColl;
	bool						mDebugGBuffer;
	bool						mDebugBVH;

	Resource::LevelInfo			mLevel;
	BoxCollider2D				mFloorCollider;

	BaseScene();
	virtual ~BaseScene();

	void SetStaticMemory(void* staticMemory, size_t size);
	virtual void VFixedUpdate(double milliseconds) = 0;

	// Common methods
	void RenderFPSIndicator();
	void RenderBVHTree();
	void RenderIMGUI(void(*)(BaseScene*) = nullptr);
};

