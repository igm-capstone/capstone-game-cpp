#pragma once
#include "Scenes/BaseScene.h"
#include "capstone-game-cpp/CollisionManager.h"
#include <Rig3D/Graphics/Camera.h>
#include <Uniforms.h>
#include <Vertex.h>

class Level01 : public BaseScene
{
	// Cbuffer data
	CBuffer::Model		mModel;
	CBuffer::LightPVM	mLightPVM;			// Used for spotlight matrices
	CBuffer::Light		mLightData;			// Used for spotlight data (color, angle, etc).
	CBuffer::SpriteSheet mSpriteSheetData;

	// Allocators
	LinearAllocator		mAllocator;
	
	// Counts
	uint32_t			mWallCount0;
	uint32_t			mPlaneCount;
	uint32_t			mSpotLightCount;
	uint32_t			mExplorerCount;

	float				mPlaneWidth;
	float				mPlaneHeight;

	// Sprite Data
	Sprite				mSpriteInstanceData[MAX_SPRITES];

	// Wall Data
	mat4f*				mWallWorldMatrices0;
	mat4f*				mPlaneWorldMatrices;

	// Lights
	mat4f*				mSpotLightWorldMatrices;
	mat4f*				mSpotLightVPTMatrices;

	// Mesh
	IMesh*				mWallMesh0;
	IMesh*				mPlaneMesh;
	IMesh*				mExplorerCubeMesh; //better solution when will come when we start to handle models
	IMesh*				mMinionCubeMesh; 
	IMesh*				mNDSQuadMesh;

	// RenderContext
	IRenderContext*		mGBufferContext;
	IRenderContext*		mShadowContext;
	
	// ShaderResource
	IShaderResource*	mWallShaderResource;
	IShaderResource*	mExplorerShaderResource;
	IShaderResource*	mPLVShaderResource;
	IShaderResource*	mSpritesShaderResource;

	// Managers
	CollisionManager	mCollisionManager;

public:
	Level01();
	~Level01();

	void VOnResize() override;

	void VInitialize() override;
	void InitializeAssets();
	void InitializeGeometry();
	void InitializeShaderResources();
	void InitializePlayers();
	
	void VUpdate(double milliseconds) override;
	void UpdateCamera();

	void VRender() override;
	void RenderShadowMaps();	// Not called per frame
	void RenderWalls();
	void RenderExplorers();
	void RenderSpotLightVolumes();
	void RenderFullScreenQuad();
	void RenderMinions();
	void RenderSprites();
	void VShutdown() override;
};
