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
#include <SceneObjects/Ghost.h>
#include <SceneObjects/StaticMesh.h>
#include <Components/MinionController.h>
#include <DebugRender.h>
#include <SceneObjects/Door.h>
#include <Rig3D/Graphics/DirectX11/DX11ShaderResource.h>

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

Level01::Level01() :
	mPlaneMesh(nullptr),
	mNDSQuadMesh(nullptr),
	mGBufferContext(nullptr),
	mShadowContext(nullptr),
	mGridContext(nullptr),
	mStaticMeshShaderResource(nullptr),
	mExplorerShaderResource(nullptr),
	mPLVShaderResource(nullptr), 
	mSpritesShaderResource(nullptr),
	mGridShaderResource(nullptr)
{
	mAIManager->SetAllocator(&mAllocator);
}

Level01::~Level01()
{
#ifdef _DEBUG
	ReleaseGlobals();
#endif

	mPlaneMesh->~IMesh();
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

	mStaticMeshShaderResource->~IShaderResource();
	mExplorerShaderResource->~IShaderResource();
	mPLVShaderResource->~IShaderResource();
	mSpritesShaderResource->~IShaderResource();
	mGridShaderResource->~IShaderResource();
	
	mGBufferContext->~IRenderContext();
	mShadowContext->~IRenderContext();
	mGridContext->~IRenderContext();

	ReleaseMacro(mFullSrcData);
	ReleaseMacro(mFullSrcDataSRV);
	ReleaseMacro(mSimpleSrcData);
	ReleaseMacro(mSimpleSrcDataSRV);
	ReleaseMacro(mOutputData);
	ReleaseMacro(mOutputDataCPURead);
	ReleaseMacro(mOutputDataSRV);

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

	mCollisionManager->Initialize();

	if (mNetworkManager->mMode == NetworkManager::Mode::CLIENT) {
		Packet p(PacketTypes::INIT_CONNECTION);
		mNetworkManager->mClient.SendData(&p);
	}
	else if (mNetworkManager->mMode == NetworkManager::Mode::SERVER) {
		auto ghost = Factory<Ghost>::Create();
		ghost->Spawn(this);
	}

	VOnResize();

	mState = BASE_SCENE_STATE_RUNNING;
}

void Level01::InitializeAssets()
{
	mModelManager->LoadModel<GPU::Vertex3>(kWallModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kTriangleWallModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kWallDoubleDoorModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kWallSingleDoorModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kWallSingleWindowModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kCurvedWallModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kFloorModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kDoorModelName);

	mLevel = Resource::LoadLevel("Assets/Level02.json", mAllocator);

	mFloorCollider.halfSize = mLevel.extents;
	mFloorCollider.origin = mLevel.center;

	mFloorCollider.halfSize = mLevel.extents;
	mFloorCollider.origin	= mLevel.center;

	mCameraManager->MoveCamera(mLevel.center, mLevel.center - vec3f(0.0f, 0.0f, 100.0f));

	mCollisionManager->mBVHTree.SetRootBoundingVolume(mLevel.center, mLevel.extents, mLevel.staticColliderCount);
	mAIManager->InitGrid(mLevel.center.x - mLevel.extents.x, mLevel.center.y + mLevel.extents.y, 2 * mLevel.extents.x, 2 * mLevel.extents.y);
	mCameraManager->SetLevelBounds(mLevel.center, mLevel.extents);
}

void Level01::InitializeGeometry()
{
	MeshLibrary<LinearAllocator> meshLibrary(&mAllocator);
	std::vector<GPU::Vertex3> vertices;
	std::vector<uint16_t> indices;

#ifdef _DEBUG
	CreateWireFrameRasterizerState();
	CreateColliderMesh(&mAllocator);
#endif

	// Floor

	Geometry::Plane(vertices, indices, mLevel.floorWidth, mLevel.floorHeight, 5, 5);

	meshLibrary.NewMesh(&mPlaneMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mPlaneMesh, &vertices[0], sizeof(GPU::Vertex3) * vertices.size(), sizeof(GPU::Vertex3));
	mRenderer->VSetMeshIndexBuffer(mPlaneMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Spot Light Volume 
	std::vector<GPU::Vertex1> coneVertices;

	for (Lamp& l : Factory<Lamp>())
	{
		Geometry::SpotlightCone(coneVertices, indices, 6, 1.0f, l.mLightAngle);

		meshLibrary.NewMesh(&l.mConeMesh, mRenderer);
		mRenderer->VSetMeshVertexBuffer(l.mConeMesh, &coneVertices[0], sizeof(GPU::Vertex1) * coneVertices.size(), sizeof(GPU::Vertex1));
		mRenderer->VSetMeshIndexBuffer(l.mConeMesh, &indices[0], indices.size());

		coneVertices.clear();
		indices.clear();
	}

	// Billboard Quad (Sprites)
	std::vector<GPU::VertexUV> ndsVertices;

	Geometry::NDSQuad(ndsVertices, indices);

	meshLibrary.NewMesh(&mNDSQuadMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mNDSQuadMesh, &ndsVertices[0], sizeof(GPU::VertexUV) * ndsVertices.size(), sizeof(GPU::VertexUV));
	mRenderer->VSetMeshIndexBuffer(mNDSQuadMesh, &indices[0], indices.size());
}

void Level01::InitializeShaderResources()
{
	// Allocate render context
	mRenderer->VCreateRenderContext(&mGBufferContext, &mAllocator);
	mRenderer->VCreateRenderContext(&mShadowContext, &mAllocator);
	mRenderer->VCreateRenderContext(&mGridContext, &mAllocator);

	// Shadow Maps for each light
	mRenderer->VCreateContextDepthStencilResourceTargets(mShadowContext, mLevel.lampCount, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	// Static Meshes
	{
		// Allocate shader resource
		mRenderer->VCreateShaderResource(&mStaticMeshShaderResource, &mAllocator);

		// Instance buffer data
		void*	ibStaticMeshData[] = { mLevel.staticMeshWorldMatrices, mLevel.floorWorldMatrices };
		size_t	ibStaticMeshSizes[] = { sizeof(mat4f) * mLevel.staticMeshCount, sizeof(mat4f) * mLevel.floorCount };
		size_t	ibStaticMeshStrides[] = { sizeof(mat4f), sizeof(mat4f) };
		size_t	ibStaticMeshOffsets[] = { 0, 0 };

		// Create the instance buffer
		mRenderer->VCreateDynamicShaderInstanceBuffers(mStaticMeshShaderResource, ibStaticMeshData, ibStaticMeshSizes, ibStaticMeshStrides, ibStaticMeshOffsets, 2);

		// Set data for instance buffer once
		mRenderer->VUpdateShaderInstanceBuffer(mStaticMeshShaderResource, mLevel.staticMeshWorldMatrices, ibStaticMeshSizes[0], 0);
		mRenderer->VUpdateShaderInstanceBuffer(mStaticMeshShaderResource, mLevel.floorWorldMatrices, ibStaticMeshSizes[1], 1);

		// Constant buffer data
		void*	cStaticMeshData[] = { mCameraManager->GetCBufferPersp() };
		size_t	cbStaticMeshSizes[] = { sizeof(CBuffer::Camera) };

		mRenderer->VCreateShaderConstantBuffers(mStaticMeshShaderResource, cStaticMeshData, cbStaticMeshSizes, 1);

		// Textures
		const char* filenames[] = { "Assets/tileable5d.png", "Assets/wood floor 2.png" };
		mRenderer->VAddShaderTextures2D(mStaticMeshShaderResource, filenames, 2);
		mRenderer->VAddShaderLinearSamplerState(mStaticMeshShaderResource, SAMPLER_STATE_ADDRESS_WRAP);

		mModelManager->GetModel(kWallModelName)->mMaterialIndex				= 0;
		mModelManager->GetModel(kTriangleWallModelName)->mMaterialIndex		= 0;
		mModelManager->GetModel(kWallDoubleDoorModelName)->mMaterialIndex	= 0;
		mModelManager->GetModel(kWallSingleDoorModelName)->mMaterialIndex	= 0;
		mModelManager->GetModel(kWallSingleWindowModelName)->mMaterialIndex = 0;
		mModelManager->GetModel(kCurvedWallModelName)->mMaterialIndex		= 0;
		mModelManager->GetModel(kWallModelName)->mMaterialIndex				= 0;
		mModelManager->GetModel(kFloorModelName)->mMaterialIndex			= 1;
	}

	// Explorers
	{
		mRenderer->VCreateShaderResource(&mExplorerShaderResource, &mAllocator);

		void* cbExplorerData[] = { mCameraManager->GetCBufferPersp(), &mModel, mSkinnedMeshMatrices, nullptr };
		size_t cbExplorerSizes[] = { sizeof(CBuffer::Camera), sizeof(CBuffer::Model), sizeof(mat4f) *  MAX_SKELETON_JOINTS, sizeof(vec4f)};

		mRenderer->VCreateShaderConstantBuffers(mExplorerShaderResource, cbExplorerData, cbExplorerSizes, 4);

		const char* filenames[] = { "Assets/Textures/BascMinionFull.png" };
		mRenderer->VAddShaderTextures2D(mExplorerShaderResource, filenames, 1);
		mRenderer->VAddShaderLinearSamplerState(mExplorerShaderResource, SAMPLER_STATE_ADDRESS_WRAP);
	}

	// PVL
	{
		mRenderer->VCreateShaderResource(&mPLVShaderResource, &mAllocator);

		void* cbPVLData[] = { &mLightData, mCameraManager->GetOrigin().pCols };
		size_t cbPVLSizes[] = { sizeof(CBuffer::Light), sizeof(vec4f) };

		mRenderer->VCreateShaderConstantBuffers(mPLVShaderResource, cbPVLData, cbPVLSizes, 2);
		mRenderer->VAddShaderLinearSamplerState(mPLVShaderResource, SAMPLER_STATE_ADDRESS_BORDER, const_cast<float*>(Colors::whiteAlpha.pCols));
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
		size_t	ibSpriteSizes[] = { sizeof(GPU::Sprite) * MAX_SPRITES };
		size_t	ibSpriteStrides[] = { sizeof(GPU::Sprite) };
		size_t	ibSpriteOffsets[] = { 0 };

		// Create the instance buffer
		mRenderer->VCreateDynamicShaderInstanceBuffers(mSpritesShaderResource, ibSpriteData, ibSpriteSizes, ibSpriteStrides, ibSpriteOffsets, 1);
	}

	// Grid CS
	{
		mRenderer->VCreateShaderResource(&mGridShaderResource, &mAllocator);

		CBuffer::GridData		mGridData;
		mGridData.gridNumCols = mAIManager->mGrid.mNumCols;
		mGridData.gridNumRows = mAIManager->mGrid.mNumRows;
		mGridData.mapTexWidth = int(mLevel.extents.x*GRID_MAP_SCALE);
		mGridData.maxTexHeight = int(mLevel.extents.y*GRID_MAP_SCALE);

		// Grid
		mRenderer->VCreateContextResourceTargets(mGridContext, 1, mGridData.mapTexWidth, mGridData.maxTexHeight);
		mRenderer->VCreateContextDepthStencilResourceTargets(mGridContext, 1, mGridData.mapTexWidth, mGridData.maxTexHeight);

		void*  cbGridData[] = { mCameraManager->GetCBufferPersp(), &mGridData, nullptr };
		size_t cbGridSizes[] = { sizeof(CBuffer::Camera), sizeof(CBuffer::GridData), sizeof(CBuffer::ObjectType) }; // 3 is for VS, not CS

		mRenderer->VCreateShaderConstantBuffers(mGridShaderResource, cbGridData, cbGridSizes, 3);

		{ //Full Grid
			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = mAIManager->mGrid.Count() * sizeof(GPU::Node);
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(GPU::Node);

			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = mAIManager->mGrid.pList;

			mDevice->CreateBuffer(&descGPUBuffer, &InitData, &mFullSrcData);

			D3D11_SHADER_RESOURCE_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descView.BufferEx.FirstElement = 0;
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.BufferEx.NumElements = mAIManager->mGrid.Count();

			mDevice->CreateShaderResourceView(mFullSrcData, &descView, &mFullSrcDataSRV);
		}

		{ //Simple Grid
			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = mAIManager->mGrid.Count() * sizeof(GPU::SimpleNode);
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(GPU::SimpleNode);

			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = mAIManager->mGrid.pList;

			mDevice->CreateBuffer(&descGPUBuffer, &InitData, &mSimpleSrcData);

			D3D11_SHADER_RESOURCE_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descView.BufferEx.FirstElement = 0;
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.BufferEx.NumElements = mAIManager->mGrid.Count();

			mDevice->CreateShaderResourceView(mSimpleSrcData, &descView, &mSimpleSrcDataSRV);
		}

		{ //Output
			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			descGPUBuffer.ByteWidth = mAIManager->mGrid.Count() * sizeof(GPU::SimpleNode);
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(GPU::SimpleNode);
			mDevice->CreateBuffer(&descGPUBuffer, NULL, &mOutputData);

			descGPUBuffer.Usage = D3D11_USAGE_STAGING;
			descGPUBuffer.BindFlags = 0;
			descGPUBuffer.MiscFlags = 0;
			descGPUBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			mDevice->CreateBuffer(&descGPUBuffer, NULL, &mOutputDataCPURead);

			D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			descView.Buffer.NumElements = mAIManager->mGrid.Count();

			mDevice->CreateUnorderedAccessView(mOutputData, &descView, &mOutputDataSRV);
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

	for (auto& mc : Factory<MinionController>())
	{
		mc.Update(milliseconds);
	}

	for (auto& gc : Factory<GhostController>())
	{
		gc.Update(milliseconds);
	}

	for (auto& dc : Factory<DominationPointController>())
	{
		dc.Update(float(milliseconds));
	}

	for (auto& skill : Factory<Skill>())
	{
		skill.Update();
	}

	for (AnimationController& ac : Factory<AnimationController>())
	{
		ac.Update(milliseconds);
	}

	if (mInput->GetKeyDown(KEYCODE_F3))
	{
		mDebugGrid = !mDebugGrid;
	}

	if (mInput->GetKeyDown(KEYCODE_F4))
	{
		mDebugColl = !mDebugColl;
	}

	if (mInput->GetKeyDown(KEYCODE_F5))
	{
		mCameraManager->mIsOrto = !mCameraManager->mIsOrto;
	}

	mCollisionManager->Update(milliseconds);
}

void Level01::VFixedUpdate(double milliseconds)
{
	if (mAIManager->IsGridDirty()) ComputeGrid(); 
	mAIManager->Update();

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

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, Colors::whiteAlpha.pCols, 0xffffffff);

	RenderStaticMeshes();

#ifdef _DEBUG
	if (mDebugColl)
	RenderWallColliders(mExplorerShaderResource, mCameraManager, &mModel);
#endif
	
	RenderDoors();
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
	mRenderer->VSetInputLayout(mApplication->mVSDefSingleMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefSingleMaterial);
	mRenderer->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);

	// Identity here just to be compatible with shader.
	mLightPVM.view = mat4f(1.0f);	

	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		mRenderer->VSetRenderContextDepthTarget(mShadowContext, i);
		mRenderer->VClearDepthStencil(mShadowContext, i, 1.0f, 0);

		// Set projection matrix for light frustum
		mLightPVM.projection = mLevel.lampVPTMatrices[i].transpose();
		
		// Create frustum object for culling.
		Rig3D::Frustum frustum;
		Rig3D::ExtractNormalizedFrustumLH(&frustum, mLevel.lampVPTMatrices[i]);

		// Storage for the indices of objects we will draw
		std::vector<uint32_t> indices;

		// Walls

		CullWalls(frustum, indices);

		auto cluster = mModelManager->GetModel("Wall");
		mRenderer->VBindMesh(cluster->mMesh);

		for (uint32_t j : indices)
		{

			mLightPVM.world = mLevel.staticMeshWorldMatrices[j];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM, 0);
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM.world, 1);

			mRenderer->VSetVertexShaderConstantBuffers(mExplorerShaderResource);
			mRenderer->VDrawIndexed(0, cluster->mMesh->GetIndexCount());
		}

		indices.clear();

		// Planes

		CullPlanes(frustum, indices, mLevel.floorWorldMatrices, mLevel.floorWidth, mLevel.floorHeight, mLevel.floorCount);

		mRenderer->VBindMesh(mPlaneMesh);
		
		for (uint32_t j : indices)
		{
			mLightPVM.world = mLevel.floorWorldMatrices[j];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM.world, 1);
			mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
			mRenderer->VDrawIndexed(0, mPlaneMesh->GetIndexCount());
		}

		i++;
	}
}

void Level01::RenderStaticMeshes()
{
	mRenderer->SetViewport();
	mRenderer->VSetRenderContextTargetsWithDepth(mGBufferContext, 0);

	mRenderer->VClearContextTarget(mGBufferContext, 0, Colors::magentaAlpha.pCols);	// Position
	mRenderer->VClearContextTarget(mGBufferContext, 1, Colors::magentaAlpha.pCols);	// Normal
	mRenderer->VClearContextTarget(mGBufferContext, 2, Colors::magentaAlpha.pCols);	// Color
	mRenderer->VClearDepthStencil(mGBufferContext, 0, 1.0f, 0);					// Depth

	mRenderer->VSetInputLayout(mApplication->mVSDefInstancedMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefInstancedMaterial);
	mRenderer->VSetPixelShader(mApplication->mPSDefMaterial);

	mRenderer->VUpdateShaderConstantBuffer(mStaticMeshShaderResource, mCameraManager->GetCBufferPersp(), 0);

	mRenderer->VSetVertexShaderConstantBuffer(mStaticMeshShaderResource, 0, 0);
	mRenderer->VSetVertexShaderInstanceBuffer(mStaticMeshShaderResource, 0, 1);
	mRenderer->VSetPixelShaderSamplerStates(mStaticMeshShaderResource);

	int instanceCount = 0;
	for (Factory<StaticMesh>::iterator it = Factory<StaticMesh>().begin(); it != Factory<StaticMesh>().end();)
	{
		auto& staticMesh = *it;
		auto modelCluster = staticMesh.mModel;
		auto numElements = modelCluster->ShareCount();
	
		mRenderer->VSetPixelShaderResourceView(mStaticMeshShaderResource, modelCluster->mMaterialIndex, 0);

		mRenderer->VBindMesh(modelCluster->mMesh);
		mRenderer->GetDeviceContext()->DrawIndexedInstanced(modelCluster->mMesh->GetIndexCount(), numElements, 0, 0, instanceCount);
		instanceCount += numElements;

		for (auto i = 0; i < numElements; i++) ++it;
	}
}

void Level01::RenderDoors()
{
	mRenderer->VSetInputLayout(mApplication->mVSDefSingleColor);
	mRenderer->VSetVertexShader(mApplication->mVSDefSingleColor);
	mRenderer->VSetPixelShader(mApplication->mPSDefColor);

	vec4f color[1] = { Colors::yellow };

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, color, 3);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 3, 2);

	ModelCluster* model = nullptr;

	for (Door& d : Factory<Door>())
	{
		if(!model)
		{
			model = d.mModel;
			mRenderer->VBindMesh(model->mMesh);
		}

		mModel.world = d.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);
		
		mRenderer->VDrawIndexed(0, model->mMesh->GetIndexCount());
	}
}

void Level01::RenderExplorers()
{
	// Shaders
	mRenderer->VSetInputLayout(mApplication->mVSDefSkinnedMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefSkinnedMaterial);
	mRenderer->VSetPixelShader(mApplication->mPSDefMaterial);

	// Constant buffers
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);

	// Textures
	mRenderer->VSetPixelShaderResourceView(mExplorerShaderResource, 0, 0);
	mRenderer->VSetPixelShaderSamplerStates(mExplorerShaderResource);

	for (Explorer& e : Factory<Explorer>())
	{
		mModel.world = e.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		e.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);
		
		mRenderer->VBindMesh(e.mModel->mMesh);
		mRenderer->VDrawIndexed(0, e.mModel->mMesh->GetIndexCount());
	}
}

void Level01::RenderSpotLightVolumes()
{
	mRenderer->VSetRenderContextTarget(mGBufferContext, 3);
	mRenderer->VClearContextTarget(mGBufferContext, 3, Colors::black.pCols);	// Albedo

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);

	mRenderer->VSetInputLayout(mApplication->mVSFwdSpotLightVolume);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSpotLightVolume);
	mRenderer->VSetPixelShader(mApplication->mPSFwdSpotLightVolume);

	mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, mCameraManager->GetOrigin().pCols, 1);
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);

	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		mModel.world = mLevel.lampWorldMatrices[i];
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		// Set Light data
		mLightData.viewProjection	= (mLevel.lampWorldMatrices[i]).transpose();
		mLightData.color			= l.mLightColor;
		mLightData.direction		= l.mLightDirection;
		mLightData.range			= l.mLightRadius;
		mLightData.cosAngle			= cos(l.mLightAngle);

		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mLightData, 0);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VSetPixelShaderConstantBuffers(mPLVShaderResource);
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

	mRenderer->VSetInputLayout(mApplication->mVSFwdFullScreenQuad);
	mRenderer->VSetVertexShader(mApplication->mVSFwdFullScreenQuad);
	mRenderer->VSetPixelShader(mApplication->mPSFwdDeferredOutput);

	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 2, 0);		// Color
	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 3, 1);		// Albedo
	mRenderer->VSetPixelShaderDepthResourceView(mGBufferContext, 0, 2);	// Depth

	mRenderer->GetDeviceContext()->Draw(3, 0);
}

void Level01::RenderMinions()
{
	mRenderer->VSetInputLayout (mApplication->mVSDefSkinnedMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefSkinnedMaterial);
	mRenderer->VSetPixelShader (mApplication->mPSDefMaterial);

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
	mRenderer->VSetPixelShaderResourceView(mExplorerShaderResource, 0, 0);
	mRenderer->VSetPixelShaderSamplerStates(mExplorerShaderResource);

	for (Minion& m : Factory<Minion>())
	{
		mModel.world = m.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		m.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);

		mRenderer->VBindMesh(m.mModel->mMesh);
		mRenderer->VDrawIndexed(0, m.mModel->mMesh->GetIndexCount());
	}
}

void Level01::RenderSprites()
{
	mRenderer->VSetContextTarget();
	auto currentTexture = -1;

	mRenderer->VSetInputLayout(mApplication->mVSFwdSprites);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSprites);
	mRenderer->VSetPixelShader(mApplication->mPSFwd2DTexture);

	mRenderer->VUpdateShaderConstantBuffer(mSpritesShaderResource, mCameraManager->GetCBufferScreenOrto(), 0);
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

	mRenderer->VUpdateShaderInstanceBuffer(mSpritesShaderResource, &mSpriteInstanceData, sizeof(GPU::Sprite) * sCount, 0);

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
	if (!mDebugGrid) return;
	for (auto i = 0; i < mAIManager->mGrid.mNumRows; i++)
		for (auto j = 0; j < mAIManager->mGrid.mNumCols; j++)
		{
			auto n = mAIManager->mGrid(i, j);
			vec4f c;
			if (n.weight == -1 && !n.hasLight) continue;
			
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
	if (mNetworkManager->mMode != NetworkManager::SERVER) return;

	// Prepare visual data (aka draw a bunch of stuff again
	mRenderer->VSetRenderContextTargetsWithDepth(mGridContext, 0);
	
	mRenderer->VClearContextTarget(mGridContext, 0, Colors::magentaAlpha.pCols);
	mRenderer->VClearDepthStencil(mGridContext, 0, 1.0f, 0);
	
	// Reset state
	mRenderer->GetDeviceContext()->RSSetState(nullptr);
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mRenderer->SetViewport(0.0f, 0.0f, static_cast<float>(mLevel.extents.x*GRID_MAP_SCALE), static_cast<float>(mLevel.extents.y*GRID_MAP_SCALE), 0.0f, 1.0f);
	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, Colors::whiteAlpha.pCols, 0xffffffff);
	
	mRenderer->VSetInputLayout(mApplication->mVSFwdInstancedColor);
	mRenderer->VSetVertexShader(mApplication->mVSFwdInstancedColor);
	mRenderer->VSetPixelShader(mApplication->mPSFwdColor);

	CBuffer::ObjectType obj;
	obj.color = vec4f(1, 1, 1, 1);

	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, mCameraManager->GetCBufferFullLevelOrto(), 0);
	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, &obj, 2);

	mRenderer->VSetVertexShaderConstantBuffer(mGridShaderResource, 0, 0);
	mRenderer->VSetVertexShaderConstantBuffer(mGridShaderResource, 2, 1);

	mRenderer->VSetVertexShaderInstanceBuffer(mStaticMeshShaderResource, 0, 1);
	
	int instanceCount = 0;
	for (Factory<StaticMesh>::iterator it = Factory<StaticMesh>().begin(); it != Factory<StaticMesh>().end();)
	{
		auto& staticMesh = *it;
		auto modelCluster = staticMesh.mModel;
		auto numElements = modelCluster->ShareCount();

		mRenderer->VBindMesh(modelCluster->mMesh);
		mRenderer->GetDeviceContext()->DrawIndexedInstanced(modelCluster->mMesh->GetIndexCount(), numElements, 0, 0, instanceCount);
		instanceCount += numElements;

		for (auto i = 0; i < numElements; i++) ++it;
	}

	mRenderer->VSetContextTarget();

	// Reset current grid to -10 / 0 on player
	mAIManager->ResetGridData();

	//Copy previous results to a CPU friendly buffer
	mDeviceContext->CopyResource(mOutputDataCPURead, mOutputData);

	//Pass 1
	mRenderer->VSetComputeShader(mApplication->mCSGridPass1);

	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, mCameraManager->GetCBufferFullLevelOrto(), 0);
	mRenderer->VSetComputeShaderConstantBuffers(mGridShaderResource);

	mDeviceContext->UpdateSubresource(mFullSrcData, 0, NULL, mAIManager->mGrid.pList, 0, 0);
	mDeviceContext->CSSetShaderResources(0, 1, &mFullSrcDataSRV);			//Full grid data
	mRenderer->VSetComputeShaderResourceView(mGBufferContext, 3, 1);		//Shadow Map
	mRenderer->VSetComputeShaderResourceView(mGridContext, 0, 2);		//Obstacles Map
	mDeviceContext->CSSetUnorderedAccessViews(0, 1, &mOutputDataSRV, NULL);	//Output

	mRenderer->GetDeviceContext()->Dispatch(mAIManager->mGrid.mNumRows / GRID_MULT_OF, mAIManager->mGrid.mNumCols / GRID_MULT_OF, 1);

	mDeviceContext->CSSetShaderResources(0, 3, mNullSRV);

	//Pass 2
	mRenderer->VSetComputeShader(mApplication->mCSGridPass2);

	mDeviceContext->CopyResource(mSimpleSrcData, mOutputData);
	mDeviceContext->CSSetShaderResources(0, 1, &mSimpleSrcDataSRV);			//Simple grid data
	mDeviceContext->CSSetUnorderedAccessViews(0, 1, &mOutputDataSRV, NULL);	//Output

	for (int i = 0; i < 30; i++) {
		mRenderer->GetDeviceContext()->Dispatch(mAIManager->mGrid.mNumRows / GRID_MULT_OF, mAIManager->mGrid.mNumCols / GRID_MULT_OF, 1);
		//mDeviceContext->CSSetShaderResources(0, 1, mNullSRV);
		mDeviceContext->CopyResource(mSimpleSrcData, mOutputData);
	}

	mDeviceContext->CSSetShader(NULL, NULL, 0);
	mDeviceContext->CSSetShaderResources(0, 3, mNullSRV);

	//Map and update
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (mDeviceContext->Map(mOutputDataCPURead, 0, D3D11_MAP_READ, 0, &mappedResource) == S_OK) {
		GPU::SimpleNode* simpleNodes = reinterpret_cast<GPU::SimpleNode*>(mappedResource.pData);
		for (auto i = 0; i < mAIManager->mGrid.Count(); i++)
		{
			auto &a = mAIManager->mGrid.pList[i];
			auto &b = simpleNodes[i];
			a.weight = b.weight;
			a.hasLight = b.hasLight;
		}
		mDeviceContext->Unmap(mOutputDataCPURead, 0);
	}
}

void Level01::VShutdown()
{

}