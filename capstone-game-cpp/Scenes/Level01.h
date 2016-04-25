#pragma once
#include "Scenes/BaseScene.h"
#include <Uniforms.h>
#include <Vertex.h>

enum GameState : char
{
	GAME_STATE_INITIAL,		// Use to check for players ready
	GAME_STATE_CAPTURE_0,
	GAME_STATE_CAPTURE_1,
	GAME_STATE_FINAL_GHOST_WIN,
	GAME_STATE_FINAL_EXPLORERS_WIN
};

class Level01 : public BaseScene
{
	// Cbuffer data
	CBuffer::Model			mModel;
	CBuffer::LightPVM		mLightPVM;			// Used for spotlight matrices
	CBuffer::Light			mLightData;			// Used for spotlight data (color, angle, etc).
	CBuffer::Effect			mTime;

	// GPU Data
	mat4f				mSkinnedMeshMatrices[MAX_SKELETON_JOINTS];

	// Allocators
	LinearAllocator		mAllocator;

	// Mesh
	IMesh*				mCubeMesh;
	IMesh*				mSphereMesh;
	IMesh*				mNDSQuadMesh;

	// RenderContext
	IRenderContext*		mGBufferContext;
	IRenderContext*		mShadowContext;
	IRenderContext*		mGridContext;
	
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

	GameState mGameState;
	char mTiersCaptured[2];	

public:
	Level01();
	~Level01();

	void VOnResize() override;

	void VInitialize() override;
	void InitializeAssets();
	void InitializeGeometry();
	void InitializeShaderResources();
	void InitializeGameState();

	void VUpdate(double milliseconds) override;
	void VFixedUpdate(double milliseconds) override;
	void UpdateGameState(double milliseconds);
	bool IsExplorerAlive();
	void SetReady(int clientID);

	void VRender() override;
	void RenderShadowMaps();	// Not called per frame
	void RenderStaticMeshes();
	void RenderDoors();
	void RenderExplorers();
	void RenderSpotLightVolumes();
	void RenderFullScreenQuad();
	void RenderMinions();
	void RenderEffects();
	void RenderHealthBars();
	void RenderSprites();
	void RenderGrid();
	void ComputeGrid();

	void VShutdown() override;

	static bool ActivationPredicate(class Explorer* explorer);
};
