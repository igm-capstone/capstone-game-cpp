#pragma once
#include "Scenes/BaseScene.h"
#include "capstone-game-cpp/CollisionManager.h"
#include <Rig3D/Graphics/Camera.h>
#include <Uniforms.h>

class Level01 : public BaseScene
{
	// Cbuffer data
	CbufferPVM			mPVM;			// Used for main camera matrics
	CbufferPVM			mLightPVM;		// Used for spotlight matrices
	CBufferLight		mLightData;		// Used for spotlight data (color, angle, etc).

	// Allocators
	LinearAllocator		mAllocator;
	
	// Counts
	uint32_t			mWallCount0;
	uint32_t			mPlaneCount;
	uint32_t			mPointLightCount;
	uint32_t			mExplorerCount;

	float				mPlaneWidth;
	float				mPlaneHeight;

	// Wall Data
	mat4f*				mWallWorldMatrices0;
	mat4f*				mPlaneWorldMatrices;
	mat4f*				mPointLightWorldMatrices;
	vec4f*				mPointLightColors;

	// Mesh
	IMesh*				mWallMesh0;
	IMesh*				mPlaneMesh;
	IMesh*				mExplorerCubeMesh; //better solution when will come when we start to handle models
	IMesh*				mPLVMesh;
	IMesh*				mNDSQuadMesh;

	// RenderContext
	IRenderContext*		mGBufferContext;
	IRenderContext*		mShadowContext;

	// ShaderResource
	IShaderResource*	mWallShaderResource;
	IShaderResource*	mExplorerShaderResource;
	IShaderResource*	mPLVShaderResource;

	// Managers
	CollisionManager	mCollisionManager;

public:
	Level01();
	~Level01();

	void VOnResize() override;

	void VInitialize() override;
	void InitializeResource();
	void InitializeGeometry();
	void InitializeShaderResources();
	void InitializePlayers();
	
	void VUpdate(double milliseconds) override;
	void UpdateCamera();

	void VRender() override;
	void RenderShadowMaps();	// Not called per frame
	void RenderWalls();
	void RenderExplorers();
	void RenderPointLightVolumes();
	void RenderFullScreenQuad();

	

	void VShutdown() override;
};