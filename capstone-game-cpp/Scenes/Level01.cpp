#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Level01.h"
#include <Resource.h>
#include <Colors.h>
#include <Vertex.h>
#include <Rig3D/Geometry.h>
#include <Rig3D/Graphics/Interface/IShaderResource.h>
#include <SceneObjects/Explorer.h>
#include <Rig3D/Graphics/Interface/IRenderContext.h>
#include <FBXResource.h>
#include <Culler.h>
#include <SceneObjects/Lamp.h>
#include <SceneObjects/Minion.h>
#include <trace.h>
#include <Components/DominationPointController.h>
#include <Components/ExplorerController.h>
#include <Components/Skill.h>
#include <Components/AnimationController.h>
#include <Components/Health.h>
#include <Components/GhostController.h>

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

Level01::Level01() :
	mWallCount0(0),
	mPlaneCount(0),
	mSpotLightCount(0),
	mExplorerCount(0),
	mPlaneWidth(0.0f),
	mPlaneHeight(0.0f),
	mWallWorldMatrices0(nullptr),
	mPlaneWorldMatrices(nullptr),
	mSpotLightWorldMatrices(nullptr),
	mSpotLightVPTMatrices(nullptr),
	mWallMesh0(nullptr),
	mPlaneMesh(nullptr),
	mExplorerCubeMesh(nullptr),
	mMinionCubeMesh(nullptr),
	mNDSQuadMesh(nullptr),
	mGBufferContext(nullptr),
	mShadowContext(nullptr),
	mWallShaderResource(nullptr),
	mExplorerShaderResource(nullptr),
	mPLVShaderResource(nullptr), 
	mSpritesShaderResource(nullptr),
	mGridShaderResource(nullptr)
{

}

Level01::~Level01()
{
	mWallMesh0->~IMesh();
	mPlaneMesh->~IMesh();
	mExplorerCubeMesh->~IMesh();
	mMinionCubeMesh->~IMesh();
	mNDSQuadMesh->~IMesh();

	for (Lamp& l : Factory<Lamp>())
	{
		l.mConeMesh->~IMesh();
	}

	for (Explorer& e : Factory<Explorer>())
	{
		//e.mMesh->~IMesh();
	}

	for (Minion& m : Factory<Minion>())
	{
		//m.mMesh->~IMesh();
	}

	mWallShaderResource->~IShaderResource();
	mExplorerShaderResource->~IShaderResource();
	mPLVShaderResource->~IShaderResource();
	mSpritesShaderResource->~IShaderResource();
	mGridShaderResource->~IShaderResource();
	
	mGBufferContext->~IRenderContext();
	mShadowContext->~IRenderContext();

	ReleaseMacro(mSrcDataGPUBuffer);
	ReleaseMacro(mSrcDataGPUBufferView);
	ReleaseMacro(mDestDataGPUBuffer);
	ReleaseMacro(mDestDataGPUBufferCPURead);
	ReleaseMacro(mDestDataGPUBufferView);

	mAllocator.Free();
}

void Level01::VOnResize()
{
	// 0: Position, 1: Normal, 2: Color 3: Albedo
	mRenderer->VCreateContextResourceTargets(mGBufferContext, 4, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
	mRenderer->VCreateContextDepthStencilResourceTargets(mGBufferContext, 1, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

	// Camera
	mCameraManager->OnResize();
}

#pragma region Initialization

void Level01::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	mAllocator.SetMemory(mStaticMemory, mStaticMemory + mStaticMemorySize);
	mRenderer->SetDelegate(this);

	InitializeAssets();
	InitializeGeometry();
	InitializeShaderResources();
	RenderShadowMaps();

	mCollisionManager.Initialize();

	VOnResize();

	mState = BASE_SCENE_STATE_RUNNING;
}

void Level01::InitializeAssets()
{
	auto level = Resource::LoadLevel("Assets/Level01.json", mAllocator);

	mWallWorldMatrices0 = level.wallWorldMatrices;
	mWallCount0			= level.wallCount;

	mPlaneWorldMatrices = level.floorWorldMatrices;
	mPlaneWidth			= level.floorWidth;
	mPlaneHeight		= level.floorHeight;
	mPlaneCount			= level.floorCount;

	mSpotLightWorldMatrices = level.lampWorldMatrices;
	mSpotLightVPTMatrices	= level.lampVPTMatrices;
	mSpotLightCount			= level.lampCount;

	mFloorCollider.halfSize = level.extents;
	mFloorCollider.origin	= level.center;

	mAIManager.InitGrid(level.center.x - level.extents.x, level.center.y + level.extents.y, 2 * level.extents.x, 2 * level.extents.y, mAllocator);
}

void Level01::InitializeGeometry()
{
	// Wall Mesh
	MeshLibrary<LinearAllocator> meshLibrary(&mAllocator);
	meshLibrary.NewMesh(&mWallMesh0, mRenderer);

	std::vector<Vertex3> vertices;
	std::vector<uint16_t> indices;

	Geometry::Cube(vertices, indices, 2);

	mRenderer->VSetMeshVertexBuffer(mWallMesh0, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mWallMesh0, &indices[0], indices.size());

	// Explorer Mesh
	FBXMeshResource<SkinnedVertex> explorerFBXResource("Assets/AnimTest.fbx");
	meshLibrary.LoadMesh(&mExplorerCubeMesh, mRenderer, explorerFBXResource);

	for (Explorer& e : Factory<Explorer>())
	{
		e.mAnimationController->mSkeletalHierarchy	= explorerFBXResource.mSkeletalHierarchy;
		e.mAnimationController->mSkeletalAnimations = explorerFBXResource.mSkeletalAnimations;
		e.mAnimationController->PlayLoopingAnimation("Take 001");
	}


	// Minion 
	meshLibrary.NewMesh(&mMinionCubeMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mMinionCubeMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mMinionCubeMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Floor
	Geometry::Plane(vertices, indices, mPlaneWidth, mPlaneHeight, 5, 5);

	meshLibrary.NewMesh(&mPlaneMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mPlaneMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mPlaneMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Spot Light Volume 
	std::vector<Vertex1> coneVertices;

	for (Lamp& l : Factory<Lamp>())
	{
		Geometry::SpotlightCone(coneVertices, indices, 6, 1.0f, l.mLightAngle);

		meshLibrary.NewMesh(&l.mConeMesh, mRenderer);
		mRenderer->VSetMeshVertexBuffer(l.mConeMesh, &coneVertices[0], sizeof(Vertex1) * coneVertices.size(), sizeof(Vertex1));
		mRenderer->VSetMeshIndexBuffer(l.mConeMesh, &indices[0], indices.size());

		coneVertices.clear();
		indices.clear();
	}

	// Billboard Quad
	std::vector<NDSVertex> ndsVertices;

	Geometry::NDSQuad(ndsVertices, indices);

	meshLibrary.NewMesh(&mNDSQuadMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mNDSQuadMesh, &ndsVertices[0], sizeof(NDSVertex) * ndsVertices.size(), sizeof(NDSVertex));
	mRenderer->VSetMeshIndexBuffer(mNDSQuadMesh, &indices[0], indices.size());
}

void Level01::InitializeShaderResources()
{
	// Allocate render context
	mRenderer->VCreateRenderContext(&mGBufferContext, &mAllocator);
	mRenderer->VCreateRenderContext(&mShadowContext, &mAllocator);

	// Shadow Maps for each light
	mRenderer->VCreateContextDepthStencilResourceTargets(mShadowContext, mSpotLightCount, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	// Walls
	{
		// Allocate wall shader resource
		mRenderer->VCreateShaderResource(&mWallShaderResource, &mAllocator);

		// Instance buffer data
		void*	ibWallData[] = { mWallWorldMatrices0, mPlaneWorldMatrices };
		size_t	ibWallSizes[] = { sizeof(mat4f) * mWallCount0, sizeof(mat4f) * mPlaneCount };
		size_t	ibWallStrides[] = { sizeof(mat4f), sizeof(mat4f) };
		size_t	ibWallOffsets[] = { 0, 0 };

		// Create the instance buffer
		mRenderer->VCreateDynamicShaderInstanceBuffers(mWallShaderResource, ibWallData, ibWallSizes, ibWallStrides, ibWallOffsets, 2);

		// Set data for instance buffer once
		mRenderer->VUpdateShaderInstanceBuffer(mWallShaderResource, mWallWorldMatrices0, ibWallSizes[0], 0);
		mRenderer->VUpdateShaderInstanceBuffer(mWallShaderResource, mPlaneWorldMatrices, ibWallSizes[1], 1);

		// Constant buffer data
		void*	cbWallData[] = { mCameraManager->GetCBufferPersp() };
		size_t	cbWallSizes[] = { sizeof(CBuffer::Camera) };

		mRenderer->VCreateShaderConstantBuffers(mWallShaderResource, cbWallData, cbWallSizes, 1);

		// Textures
		const char* filenames[] = { "Assets/tileable5d.png", "Assets/wood floor 2.png" };
		mRenderer->VAddShaderTextures2D(mWallShaderResource, filenames, 2);
		mRenderer->VAddShaderLinearSamplerState(mWallShaderResource, SAMPLER_STATE_ADDRESS_WRAP);
	}

	// Explorers
	{
		mRenderer->VCreateShaderResource(&mExplorerShaderResource, &mAllocator);

		void* cbExplorerData[] = { mCameraManager->GetCBufferPersp(), &mModel, mSkinnedMeshMatices };
		size_t cbExplorerSizes[] = { sizeof(CBuffer::Camera), sizeof(CBuffer::Model), sizeof(mat4f) *  MAX_SKELETON_JOINTS};

		mRenderer->VCreateShaderConstantBuffers(mExplorerShaderResource, cbExplorerData, cbExplorerSizes, 3);
	}

	// PVL
	{
		mRenderer->VCreateShaderResource(&mPLVShaderResource, &mAllocator);

		void* cbPVLData[] = { &mLightData, mCameraManager->GetOrigin().pCols };
		size_t cbPVLSizes[] = { sizeof(CBuffer::Light), sizeof(vec4f) };

		mRenderer->VCreateShaderConstantBuffers(mPLVShaderResource, cbPVLData, cbPVLSizes, 2);
		mRenderer->VAddShaderLinearSamplerState(mPLVShaderResource, SAMPLER_STATE_ADDRESS_BORDER, const_cast<float*>(Colors::transparent.pCols));
		mRenderer->AddAdditiveBlendState(mPLVShaderResource);
	}

	// Sprites
	{
		mRenderer->VCreateShaderResource(&mSpritesShaderResource, &mAllocator);

		mSpriteSheetData.sliceWidth = 100 / 1;
		mSpriteSheetData.sliceHeight = 32 / 2;

		void*  cbSpritesData[] = { mCameraManager->GetCBufferPersp(), &mSpriteSheetData };
		size_t cbSpritesSizes[] = { sizeof(CBuffer::Camera), sizeof(CBuffer::SpriteSheet) };

		mRenderer->VCreateShaderConstantBuffers(mSpritesShaderResource, cbSpritesData, cbSpritesSizes, 2);
		mRenderer->VUpdateShaderConstantBuffer(mSpritesShaderResource, &mSpriteSheetData, 1);

		const char* filenames[] = { "Assets/Health.png" };
		mRenderer->VAddShaderTextures2D(mSpritesShaderResource, filenames, 1);
		mRenderer->VAddShaderPointSamplerState(mSpritesShaderResource, SAMPLER_STATE_ADDRESS_WRAP);

		// Instance buffer data
		void*	ibSpriteData[] = { &mSpriteInstanceData };
		size_t	ibSpriteSizes[] = { sizeof(Sprite) * MAX_SPRITES };
		size_t	ibSpriteStrides[] = { sizeof(Sprite) };
		size_t	ibSpriteOffsets[] = { 0 };

		// Create the instance buffer
		mRenderer->VCreateDynamicShaderInstanceBuffers(mSpritesShaderResource, ibSpriteData, ibSpriteSizes, ibSpriteStrides, ibSpriteOffsets, 1);
	}

	// Grid CS
	{
		mRenderer->VCreateShaderResource(&mGridShaderResource, &mAllocator);

		int	gridData[4] = { mAIManager.mGrid.mNumCols , mAIManager.mGrid.mNumRows, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight() };
		void*  cbGridData[] = { mCameraManager->GetCBufferPersp(), gridData };
		size_t cbGridSizes[] = { sizeof(CBuffer::Camera), sizeof(int) * 4 };

		mRenderer->VCreateShaderConstantBuffers(mGridShaderResource, cbGridData, cbGridSizes, 2);

		{
			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = mAIManager.mGrid.mNumCols * mAIManager.mGrid.mNumRows * sizeof(GridVertex);
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(GridVertex);
			mDevice->CreateBuffer(&descGPUBuffer, NULL, &mDestDataGPUBuffer);

			descGPUBuffer.Usage = D3D11_USAGE_STAGING;
			descGPUBuffer.BindFlags = 0;
			descGPUBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			mDevice->CreateBuffer(&descGPUBuffer, NULL, &mDestDataGPUBufferCPURead);

			D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			descView.Buffer.NumElements = mAIManager.mGrid.mNumCols * mAIManager.mGrid.mNumRows;

			mDevice->CreateUnorderedAccessView(mDestDataGPUBuffer, &descView, &mDestDataGPUBufferView);
		}

		{
			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = mAIManager.mGrid.mNumCols * mAIManager.mGrid.mNumRows * sizeof(GridVertex);
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(Node);

			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = mAIManager.mGrid.pList;

			mDevice->CreateBuffer(&descGPUBuffer, &InitData, &mSrcDataGPUBuffer);

			D3D11_SHADER_RESOURCE_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descView.BufferEx.FirstElement = 0;
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.BufferEx.NumElements = mAIManager.mGrid.mNumCols * mAIManager.mGrid.mNumRows;

			mDevice->CreateShaderResourceView(mSrcDataGPUBuffer, &descView, &mSrcDataGPUBufferView);
		}
	}
}
#pragma endregion

#pragma region Update

void Level01::VUpdate(double milliseconds)
{
	// TO DO: Possibly a Components Update method... if we move this code to application level.
	for (ExplorerController& ec : Factory<ExplorerController>())
	{
		ec.Update(milliseconds);
	}

	for (auto& gc : Factory<GhostController>())
	{
		gc.Update(milliseconds);
	}

	for (auto& dc : Factory<DominationPointController>())
	{
		dc.Update(milliseconds);
	}


	for (auto& skill : Factory<Skill>())
	{
		skill.Update();
	}

	for (AnimationController& ac : Factory<AnimationController>())
	{
		ac.Update(milliseconds);
	}

	// TO DO: Wrap in a collision manager update.
	mCollisionManager.DetectCollisions();
	mCollisionManager.ResolveCollisions();

	ComputeGrid();
	mAIManager.Update();

	mNetworkManager->Update();
}
#pragma endregion

#pragma region Render

void Level01::VRender()
{
	// Reset state.
	mRenderer->GetDeviceContext()->RSSetState(nullptr);
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mRenderer->SetViewport();

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, Colors::transparent.pCols, 0xffffffff);

	RenderWalls();
	RenderExplorers();
	RenderMinions();
	RenderSpotLightVolumes();
	RenderFullScreenQuad();
	RenderIMGUI(); 
	RenderSprites();
	
	mRenderer->GetDeviceContext()->PSSetShaderResources(0, 4, mNullSRV);

	RenderGrid();
	RENDER_TRACE();
	mRenderer->VSwapBuffers();
}

void Level01::RenderShadowMaps()
{
	mRenderer->SetViewport(0.0f, 0.0f, static_cast<float>(SHADOW_MAP_SIZE), static_cast<float>(SHADOW_MAP_SIZE), 0.0f, 1.0f);
	mRenderer->VSetInputLayout(mApplication->mExplorerVertexShader);
	mRenderer->VSetVertexShader(mApplication->mExplorerVertexShader);
	mRenderer->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);

	// Identity here just to be compatible with shader.
	mLightPVM.view = mat4f(1.0f);	

	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		mRenderer->VSetRenderContextDepthTarget(mShadowContext, i);
		mRenderer->VClearDepthStencil(mShadowContext, i, 1.0f, 0);

		// Set projection matrix for light frustum
		mLightPVM.projection = mSpotLightVPTMatrices[i].transpose();
		
		std::vector<uint32_t> indices;
		Rig3D::Frustum frustum;
		Rig3D::ExtractNormalizedFrustumLH(&frustum, mSpotLightVPTMatrices[i]);

		// Walls

		CullWalls(frustum, indices);

		mRenderer->VBindMesh(mWallMesh0);

		for (uint32_t j : indices)
		{
			mLightPVM.world = mWallWorldMatrices0[j];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM, 0);
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM.world, 1);

			mRenderer->VSetVertexShaderConstantBuffers(mExplorerShaderResource);
			mRenderer->VDrawIndexed(0, mWallMesh0->GetIndexCount());
		}

		indices.clear();

		// Planes

		CullPlanes(frustum, indices, mPlaneWorldMatrices, mPlaneWidth, mPlaneHeight, mPlaneCount);

		mRenderer->VBindMesh(mPlaneMesh);
		
		for (uint32_t j : indices)
		{
			mLightPVM.world = mPlaneWorldMatrices[j];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM.world, 1);
			mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
			mRenderer->VDrawIndexed(0, mPlaneMesh->GetIndexCount());
		}

		i++;
	}
}

void Level01::RenderWalls()
{
	mRenderer->SetViewport();
	mRenderer->VSetRenderContextTargetsWithDepth(mGBufferContext, 0);

	mRenderer->VClearContextTarget(mGBufferContext, 0, Colors::magenta.pCols);	// Position
	mRenderer->VClearContextTarget(mGBufferContext, 1, Colors::magenta.pCols);	// Normal
	mRenderer->VClearContextTarget(mGBufferContext, 2, Colors::magenta.pCols);	// Color
	mRenderer->VClearDepthStencil(mGBufferContext, 0, 1.0f, 0);					// Depth

	mRenderer->VSetInputLayout(mApplication->mQuadVertexShader);
	mRenderer->VSetVertexShader(mApplication->mQuadVertexShader);
	mRenderer->VSetPixelShader(mApplication->mQuadPixelShader);

	// This can probably go into the render method...
	mRenderer->VUpdateShaderConstantBuffer(mWallShaderResource, mCameraManager->GetCBufferPersp(), 0);

	mRenderer->VBindMesh(mWallMesh0);
	mRenderer->VSetVertexShaderInstanceBuffer(mWallShaderResource, 0, 1);
	mRenderer->VSetVertexShaderConstantBuffer(mWallShaderResource, 0, 0);
	mRenderer->VSetPixelShaderResourceView(mWallShaderResource, 0, 0);
	mRenderer->VSetPixelShaderSamplerStates(mWallShaderResource);

	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mWallMesh0->GetIndexCount(), mWallCount0, 0, 0, 0);

	mRenderer->VBindMesh(mPlaneMesh);
	mRenderer->VSetVertexShaderInstanceBuffer(mWallShaderResource, 1, 1);
	mRenderer->VSetPixelShaderResourceView(mWallShaderResource, 1, 0);

	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mPlaneMesh->GetIndexCount(), mPlaneCount, 0, 0, 0);
}

void Level01::RenderExplorers()
{
	mRenderer->VSetInputLayout(mApplication->mSkinnedVertexShader);
	mRenderer->VSetVertexShader(mApplication->mSkinnedVertexShader);
	mRenderer->VSetPixelShader(mApplication->mExplorerPixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
	
	for (Explorer& e : Factory<Explorer>())
	{
		mModel.world = e.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		e.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);

		mRenderer->VBindMesh(mExplorerCubeMesh);
		mRenderer->VDrawIndexed(0, mExplorerCubeMesh->GetIndexCount());
	}
}

void Level01::RenderSpotLightVolumes()
{
	mRenderer->VSetRenderContextTarget(mGBufferContext, 3);
	mRenderer->VClearContextTarget(mGBufferContext, 3, Colors::black.pCols);	// Albedo

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);

	mRenderer->VSetInputLayout(mApplication->mPLVolumeVertexShader);
	mRenderer->VSetVertexShader(mApplication->mPLVolumeVertexShader);
	mRenderer->VSetPixelShader(mApplication->mPLVolumePixelShader);

	

	mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, mCameraManager->GetOrigin().pCols, 1);

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		mModel.world = mSpotLightWorldMatrices[i];
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		// Set Light data
		mLightData.viewProjection	= (mSpotLightVPTMatrices[i]).transpose();
		mLightData.color			= l.mLightColor;
		mLightData.direction		= l.mLightDirection;
		mLightData.range			= l.mLightRadius;
		mLightData.cosAngle			= cos(l.mLightAngle);

		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mLightData, 0);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
		mRenderer->VSetPixelShaderConstantBuffers(mPLVShaderResource);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);
		mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 0, 0);
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 0, 0);		// Position
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 1, 1);		// Normal
		mRenderer->VSetPixelShaderDepthResourceView(mShadowContext, i, 2);	// Shadow
		mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);		// Border
		
		mRenderer->VBindMesh(l.mConeMesh);

		mRenderer->VDrawIndexed(0, l.mConeMesh->GetIndexCount());
		i++;
	}

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, color, 0xffffffff);
}

void Level01::RenderFullScreenQuad()
{
	mRenderer->VSetContextTarget();
	mRenderer->VClearContextTarget(Colors::magenta.pCols);

	mRenderer->VSetInputLayout(mApplication->mNDSQuadVertexShader);
	mRenderer->VSetVertexShader(mApplication->mNDSQuadVertexShader);
	mRenderer->VSetPixelShader(mApplication->mNDSQuadPixelShader);

	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 2, 0);		// Color
	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 3, 1);		// Albedo
	mRenderer->VSetPixelShaderDepthResourceView(mGBufferContext, 0, 2);	// Depth

	mRenderer->VBindMesh(mNDSQuadMesh);
	mRenderer->VDrawIndexed(0, mNDSQuadMesh->GetIndexCount());
}

void Level01::RenderMinions()
{
	mRenderer->VSetInputLayout (mApplication->mExplorerVertexShader);
	mRenderer->VSetVertexShader(mApplication->mExplorerVertexShader);
	mRenderer->VSetPixelShader (mApplication->mExplorerPixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
	for (Minion& m : Factory<Minion>())
	{
		mModel.world = m.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		mRenderer->VBindMesh(mMinionCubeMesh);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);
		mRenderer->VDrawIndexed(0, mMinionCubeMesh->GetIndexCount());
	}
}

void Level01::RenderSprites()
{
	mRenderer->VSetContextTarget();
	auto currentTexture = -1;

	mRenderer->VSetInputLayout(mApplication->mSpriteVertexShader);
	mRenderer->VSetVertexShader(mApplication->mSpriteVertexShader);
	mRenderer->VSetPixelShader(mApplication->mSpritePixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mSpritesShaderResource, mCameraManager->GetCBufferOrto(), 0);
	mRenderer->VSetVertexShaderConstantBuffers(mSpritesShaderResource);
	UINT sCount = 0;
	for (Health& h : Factory<Health>())
	{
		mSpriteInstanceData[sCount].pointpos = mCameraManager->World2Screen(h.mSceneObject->mTransform->GetPosition()) + vec2f(0, -32);
		mSpriteInstanceData[sCount].id = 1;
		mSpriteInstanceData[sCount].size = { 100, 16 };
		mSpriteInstanceData[sCount].scale = { h.GetHealthPerc(), 1 };
		sCount++;

		mSpriteInstanceData[sCount].pointpos = mSpriteInstanceData[sCount - 1].pointpos;
		mSpriteInstanceData[sCount].id = 0;
		mSpriteInstanceData[sCount].size = { 100, 16 };
		mSpriteInstanceData[sCount].scale = { 1, 1 };
		sCount++;
	}

	mRenderer->VUpdateShaderInstanceBuffer(mSpritesShaderResource, &mSpriteInstanceData, sizeof(Sprite) * sCount, 0);

	mRenderer->VSetVertexShaderResourceView(mSpritesShaderResource, 0, 0);
	mRenderer->VSetPixelShaderResourceView(mSpritesShaderResource, 0, 0);
	mRenderer->VSetPixelShaderSamplerStates(mSpritesShaderResource);

	mRenderer->VBindMesh(mNDSQuadMesh);
	mRenderer->VSetVertexShaderInstanceBuffer(mSpritesShaderResource, 0, 1);
	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mNDSQuadMesh->GetIndexCount(), sCount, 0, 0, 0);
}

void Level01::RenderGrid()
{
#ifdef _DEBUG
	for (auto i = 0; i < mAIManager.mGrid.mNumRows; i++)
		for (auto j = 0; j < mAIManager.mGrid.mNumCols; j++)
		{
			auto n = mAIManager.mGrid(i, j);
			vec4f c;
			switch ((int)n.weight) {
			case -10:
				c = Colors::magenta;
				break;
			case -2:
				c = Colors::red;
				break;
			case -1:
				c = Colors::yellow;
				break;
			case 0:
				c = Colors::green;
				break;
			default:
				c = vec4f(0, 0, n.weight*0.1f, 1);
				break;
			}
			TRACE_SMALL_BOX(n.worldPos, c * vec4f(1, 1, 1, 0.4f));
			if (n.hasLight) { TRACE_SMALL_CROSS(n.worldPos, Colors::yellow * vec4f(1, 1, 1, 0.4f)); }
		}
#endif
}
#pragma endregion

void Level01::ComputeGrid()
{
	mAIManager.ResetGridData();
	
	mRenderer->VSetComputeShader(mApplication->mGridComputeShader);

	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetComputeShaderConstantBuffers(mGridShaderResource);
	
	mDeviceContext->UpdateSubresource(mSrcDataGPUBuffer, 0, NULL, mAIManager.mGrid.pList, 0, 0);
	mDeviceContext->CSSetShaderResources(0, 1, &mSrcDataGPUBufferView);
	mRenderer->VSetComputeShaderResourceView(mGBufferContext, 3, 1);
	mRenderer->VSetComputeShaderResourceView(mGBufferContext, 3, 2);
	mDeviceContext->CSSetUnorderedAccessViews(0, 1, &mDestDataGPUBufferView, NULL);
	//Compute 

	for (int i = 0; i < 100; i++) {
		mRenderer->GetDeviceContext()->Dispatch(mAIManager.mGrid.mNumRows / GRID_MULT_OF, mAIManager.mGrid.mNumCols / GRID_MULT_OF, 1);
		mDeviceContext->CopyResource(mSrcDataGPUBuffer, mDestDataGPUBuffer);
	}
	mDeviceContext->CSSetShader(NULL, NULL, 0);
	mDeviceContext->CSSetShaderResources(0, 3, mNullSRV);
	
	//Copy results to a CPU friendly buffer
	mDeviceContext->CopyResource(mDestDataGPUBufferCPURead, mDestDataGPUBuffer);

	//Map and update
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	mDeviceContext->Map(mDestDataGPUBufferCPURead, 0, D3D11_MAP_READ, 0, &mappedResource);
	GridVertex* ints = reinterpret_cast<GridVertex*>(mappedResource.pData);
	memcpy(mAIManager.mGrid.pList, mappedResource.pData, mAIManager.mGrid.mNumCols * mAIManager.mGrid.mNumRows * sizeof(GridVertex));
	mDeviceContext->Unmap(mDestDataGPUBufferCPURead, 0);
}

void Level01::VShutdown()
{

}