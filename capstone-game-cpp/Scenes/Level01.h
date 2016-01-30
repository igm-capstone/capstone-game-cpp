#pragma once
#include "Scenes/BaseScene.h"
#include "capstone-game-cpp/CollisionManager.h"
#include <Rig3D/Graphics/Camera.h>

class Level01 : public BaseScene
{
	// Allocators
	LinearAllocator		mAllocator;
	
	// Camera
	Camera				mMainCamera;

	// Wall Data
	mat4f*				mWallWorldMatrices0;
	uint32_t			mWallCount0;
	
	// Mesh
	IMesh*				mWallMesh0;

	// ShaderResource
	IShaderResource*	mWallShaderResource;

	// Managers
	CollisionManager	mCollisionManager;

public:
	void VOnResize() override;
	
	void VInitialize() override;
	void InitializeGeometry();
	void InitializeShaderResources();
	void InitializeMainCamera();
	
	void VUpdate(double milliseconds) override;
	
	void VRender() override;
	void RenderWalls();

	void VShutdown() override;
};