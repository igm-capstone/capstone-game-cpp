#pragma once
#include "stdafx.h"

#include "Network\NetworkManager.h"
#include <Rig3D/Graphics/Camera.h>
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>

#define PI 3.14159265359f

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
	BaseSceneObject*			mMe;

	uint8_t*					mStaticMemory;
	size_t						mStaticMemorySize;

public:
	BaseSceneState				mState;
	Camera						mCamera;

	// Miscelanious
	BoxCollider					mFloorCollider;

	BaseScene();
	virtual ~BaseScene();

	void SetStaticMemory(void* staticMemory, size_t size);

	// Common methods
	void RenderFPSIndicator();

#pragma region Network Callbacks
public:
	//Server
	void CmdSpawnNewExplorer(int clientID);
	void CmdSpawnNewMinion(vec3f pos);

	//Client
	void RpcSpawnExistingExplorer(int UUID, vec3f pos);
	void RpcSpawnExistingMinion(int UUID, vec3f pos);
	
	//Both
	void GrantAuthority(int UUID);
	void SyncTransform(int UUID, vec3f pos);
#pragma endregion
};

