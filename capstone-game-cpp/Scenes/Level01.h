#pragma once
#include "Scenes/BaseScene.h"
#include "capstone-game-cpp/CollisionManager.h"
#include <Uniforms.h>
#include <Vertex.h>
#include <AIManager.h>
#include <ModelManager.h>

class Level01 : public BaseScene
{
	// Cbuffer data
	CBuffer::Model			mModel;
	CBuffer::LightPVM		mLightPVM;			// Used for spotlight matrices
	CBuffer::Light			mLightData;			// Used for spotlight data (color, angle, etc).
	CBuffer::SpriteSheet	mSpriteSheetData;

	// GPU Data
	GPU::Sprite			mSpriteInstanceData[MAX_SPRITES];
	mat4f				mSkinnedMeshMatices[MAX_SKELETON_JOINTS];

	// Allocators
	LinearAllocator		mAllocator;

	// Mesh
	IMesh*				mPlaneMesh;
	IMesh*				mNDSQuadMesh;

	// RenderContext
	IRenderContext*		mGBufferContext;
	IRenderContext*		mShadowContext;

	ID3D11RasterizerState* mCWRasterState; //For LHS handling.
	
	// ShaderResource
	IShaderResource*	mStaticMeshShaderResource;
	IShaderResource*	mExplorerShaderResource;
	IShaderResource*	mPLVShaderResource;
	IShaderResource*	mSpritesShaderResource;
	IShaderResource*	mGridShaderResource;
	
	ID3D11ShaderResourceView* mNullSRV[4] = { nullptr, nullptr, nullptr, nullptr };

	// Grid Compute Shader (not handled by ShaderResource, this is where it is not currently helpful)
	ID3D11Buffer*				mFullSrcData;
	ID3D11ShaderResourceView*	mFullSrcDataSRV;
	ID3D11Buffer*				mSimpleSrcData;
	ID3D11ShaderResourceView*	mSimpleSrcDataSRV;
	ID3D11Buffer*				mOutputData;
	ID3D11Buffer*				mOutputDataCPURead;
	ID3D11UnorderedAccessView*	mOutputDataSRV;
	int lastUpdate = 0;

	// Managers
	CollisionManager	mCollisionManager;
	AIManager			mAIManager;

	// Manager alias
	ModelManager*		mModelManager;

public:
	Level01();
	~Level01();

	void VOnResize() override;

	void VInitialize() override;
	void InitializeAssets();
	void InitializeGeometry();
	void InitializeShaderResources();
	
	void VUpdate(double milliseconds) override;

	void VRender() override;
	void RenderShadowMaps();	// Not called per frame
	void RenderStaticMeshes();
	void RenderWalls();
	void RenderExplorers();
	void RenderSpotLightVolumes();
	void RenderFullScreenQuad();
	void RenderMinions();
	void RenderSprites();
	void RenderGrid();
	void ComputeGrid();
	void VShutdown() override;
};
