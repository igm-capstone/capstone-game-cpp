#pragma once
#include "Scenes/BaseScene.h"
#include "capstone-game-cpp/CollisionManager.h"
#include <Rig3D/Graphics/Camera.h>
#include <Uniforms.h>

class Level01 : public BaseScene
{
	// Cbuffer data
	CbufferPVM			mPVM;

	// Allocators
	LinearAllocator		mAllocator;
	
	// Camera
	Camera				mMainCamera;

	// Counts
	uint32_t			mWallCount0;
	uint32_t			mExplorerCount;

	// Wall Data
	mat4f*				mWallWorldMatrices0;
	
	// Mesh
	IMesh*				mWallMesh0;
	IMesh*				mExplorerCubeMesh; //better solution when will come when we start to handle models

	// ShaderResource
	IShaderResource*	mWallShaderResource;
	IShaderResource*	mExplorerShaderResource;

	// Managers
	CollisionManager	mCollisionManager;

public:
	Level01();
	~Level01();

	void VOnResize() override;

	void VInitialize() override;
	void InitializeGeometry();
	void InitializeShaderResources();
	void InitializeMainCamera();
	void InitializePlayers();
	
	void VUpdate(double milliseconds) override;
	void UpdateCamera();

	void VRender() override;
	void RenderWalls();
	void RenderExplorers();

	void VShutdown() override;
};