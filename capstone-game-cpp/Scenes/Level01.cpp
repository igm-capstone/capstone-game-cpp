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
#include <SceneObjects/Region.h>
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
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Heal.h>
#include <SceneObjects/Trap.h>
#include <SceneObjects/StatusEffect.h>
#include <SceneObjects/DominationPoint.h>
#include <Components/DominationPointController.h>
#include <Components/ImpController.h>
#include <Components/FlyTrapController.h>
#include <Components/AbominationController.h>
#include <SceneObjects/Explosion.h>
#include <SceneObjects/Transmogrify.h>
#include <Mathf.h>

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

extern bool gDebugExplorer;

Level01::Level01() :
	mCubeMesh(nullptr),
	mSphereMesh(nullptr),
	mGeodesicSphereMesh(nullptr),
	mNDSQuadMesh(nullptr),
	mGBufferContext(nullptr),
	mShadowContext(nullptr),
	mGridContext(nullptr),
	mStaticMeshShaderResource(nullptr),
	mExplorerShaderResource(nullptr),
	mPLVShaderResource(nullptr), 
	mSpritesShaderResource(nullptr),
	mGridShaderResource(nullptr),
	mFullSrcData(nullptr),
	mFullSrcDataSRV(nullptr),
	mSimpleSrcData(nullptr),
	mSimpleSrcDataSRV(nullptr),
	mOutputData(nullptr),
	mOutputDataCPURead(nullptr),
	mOutputDataSRV(nullptr),
	mLightingWriteRS(nullptr),
	mLightingReadRS(nullptr),
	mLightingWriteDSS(nullptr),
	mLightingReadDSS(nullptr),
	mGameState(GAME_STATE_INITIAL)
{
	mAIManager->SetAllocator(&mAllocator);
}

Level01::~Level01()
{
#ifdef _DEBUG
	ReleaseGlobals();
#endif

	mCubeMesh->~IMesh();
	mSphereMesh->~IMesh();
	mGeodesicSphereMesh->~IMesh();
	mNDSQuadMesh->~IMesh();

	mGBufferContext->~IRenderContext();
	mShadowContext->~IRenderContext();
	mGridContext->~IRenderContext();

	mStaticMeshShaderResource->~IShaderResource();
	mExplorerShaderResource->~IShaderResource();
	mPLVShaderResource->~IShaderResource();
	mSpritesShaderResource->~IShaderResource();
	mGridShaderResource->~IShaderResource();
	
	ReleaseMacro(mFullSrcData);
	ReleaseMacro(mFullSrcDataSRV);
	ReleaseMacro(mSimpleSrcData);
	ReleaseMacro(mSimpleSrcDataSRV);
	ReleaseMacro(mOutputData);
	ReleaseMacro(mOutputDataCPURead);
	ReleaseMacro(mOutputDataSRV);
	ReleaseMacro(mLightingWriteRS);
	ReleaseMacro(mLightingReadRS);
	ReleaseMacro(mLightingWriteDSS);
	ReleaseMacro(mLightingReadDSS);

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

	mSpriteManager->Initialize(mNDSQuadMesh, mSpritesShaderResource);

	InitializeAssets();
	InitializeGeometry();
	InitializeRenderStates();
	InitializeShaderResources();
	RenderShadowMaps();

	mCollisionManager->Initialize();


	if (mNetworkManager->mMode == NetworkManager::Mode::CLIENT) {
		Packet p(PacketTypes::INIT_CONNECTION);
		mNetworkManager->mClient.SendData(&p);
	}
	else if (mNetworkManager->mMode == NetworkManager::Mode::SERVER) {
		Factory<Ghost>::Create()->Spawn(this);

		if (gDebugExplorer) {
			SpawnPoint& sp = *Factory<SpawnPoint>().begin();
			auto e = Factory<Explorer>::Create();
			e->DebugSpawn(sp.mTransform->GetPosition(), MyUUID::GenUUID());
		}
	}

	InitializeGameState();

	VOnResize();

	mState = BASE_SCENE_STATE_RUNNING;
}

void Level01::InitializeAssets()
{
	// Pre-load models
	for (int i = 0; i < STATIC_MESH_MODEL_COUNT; i++)
	{
		mModelManager->LoadModel<GPU::Vertex3>(kStaticMeshModelNames[i]);
	}

	mModelManager->LoadModel<GPU::SkinnedVertex>(kProfessorModelName);
	mModelManager->LoadModel<GPU::SkinnedVertex>(kSprinterModelName);
	mModelManager->LoadModel<GPU::SkinnedVertex>(kTrapperModelName);

	mModelManager->LoadModel<GPU::SkinnedVertex>(kMinionAnimModelName);
	mModelManager->LoadModel<GPU::SkinnedVertex>(kPlantModelName);
	mModelManager->LoadModel<GPU::SkinnedVertex>(kTrapModelName);

	mModelManager->LoadModel<GPU::Vertex3>(kDoorModelName);
	mModelManager->LoadModel<GPU::Vertex3>(kLanternModelName);
	
//	mModelManager->LoadModel<GPU::Vertex1>("Icosahedron");

	//mLevel = Resource::LoadLevel("Assets/Level02.json", mAllocator);
	mLevel = Resource::LoadLevel("Assets/RPI_Level.json", mAllocator);

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

	// Cube
	Rig3D::Geometry::Cube(vertices, indices, 2);

	meshLibrary.NewMesh(&mCubeMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mCubeMesh, &vertices[0], sizeof(GPU::Vertex3) * vertices.size(), sizeof(GPU::Vertex3));
	mRenderer->VSetMeshIndexBuffer(mCubeMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Sphere
	Rig3D::Geometry::Sphere(vertices, indices, 10, 10, 1.0f);

	meshLibrary.NewMesh(&mSphereMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mSphereMesh, &vertices[0], sizeof(GPU::Vertex3) * vertices.size(), sizeof(GPU::Vertex3));
	mRenderer->VSetMeshIndexBuffer(mSphereMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Spot Light Volume 
	std::vector<GPU::Vertex1> coneVertices;

	mModelManager->LoadModel<GPU::Vertex1>("Icosahedron");
	mGeodesicSphereMesh = mModelManager->GetModel("Icosahedron")->mMesh;

	//Geometry::Sphere(coneVertices, indices, 10, 10, 1.0f);
	//meshLibrary.NewMesh(&mGeodesicSphereMesh, mRenderer);
	//mRenderer->VSetMeshVertexBuffer(mGeodesicSphereMesh, &coneVertices[0], sizeof(GPU::Vertex1) * coneVertices.size(), sizeof(GPU::Vertex1));
	//mRenderer->VSetMeshIndexBuffer(mGeodesicSphereMesh, &indices[0], indices.size());

	coneVertices.clear();
	indices.clear();

	// Billboard Quad (Sprites)
	std::vector<GPU::VertexUV> ndsVertices;

	Geometry::NDSQuad(ndsVertices, indices);

	meshLibrary.NewMesh(&mNDSQuadMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mNDSQuadMesh, &ndsVertices[0], sizeof(GPU::VertexUV) * ndsVertices.size(), sizeof(GPU::VertexUV));
	mRenderer->VSetMeshIndexBuffer(mNDSQuadMesh, &indices[0], indices.size());
}

void Level01::InitializeRenderStates()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthClipEnable = true;

	mRenderer->GetDevice()->CreateRasterizerState(&rasterizerDesc, &mLightingWriteRS);

	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.DepthClipEnable = true;

	mRenderer->GetDevice()->CreateRasterizerState(&rasterizerDesc, &mLightingReadRS);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	// Default depth state
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil state
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask =  D3D11_DEFAULT_STENCIL_WRITE_MASK;

	// Back face stencil
	// Configure the stencil operation for the back facing polygons to increment the value in the stencil buffer 
	// when the depth test fails but to keep it unchanged when either depth test or stencil test succeed.
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	// Front face stencil
	// Configure the stencil operation for the front facing polygons to decrement the value in the stencil buffer 
	// when the depth test fails but to keep it unchanged when either depth test or stencil test succeed.
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	mRenderer->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &mLightingWriteDSS);

//	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	// Default depth state
	depthStencilDesc.DepthEnable = false;

	// Stencil state
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask =  D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	mRenderer->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &mLightingReadDSS);
}

void Level01::InitializeShaderResources()
{
	// Allocate render context
	mRenderer->VCreateRenderContext(&mGBufferContext, &mAllocator);
	mRenderer->VCreateRenderContext(&mShadowContext, &mAllocator);
	mRenderer->VCreateRenderContext(&mGridContext, &mAllocator);

	// Shadow Maps for each light
	mRenderer->VCreateContextCubicShadowTargets(mShadowContext, mLevel.lampCount, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	mRenderer->VCreateContextDepthStencilTargets(mShadowContext, 1, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	// Static Meshes
	{
		// Allocate shader resource
		mRenderer->VCreateShaderResource(&mStaticMeshShaderResource, &mAllocator);

		// Instance buffer data
		void*	ibStaticMeshData[] = { mLevel.staticMeshWorldMatrices, &mLevel.materialIDs[0] };
		size_t	ibStaticMeshSizes[] = { sizeof(mat4f) * mLevel.staticMeshCount, sizeof(uint32_t) * mLevel.materialIDs.size() };
		size_t	ibStaticMeshStrides[] = { sizeof(mat4f), sizeof(uint32_t) };
		size_t	ibStaticMeshOffsets[] = { 0, 0 };

		// Create the instance buffer
		mRenderer->VCreateDynamicShaderInstanceBuffers(mStaticMeshShaderResource, ibStaticMeshData, ibStaticMeshSizes, ibStaticMeshStrides, ibStaticMeshOffsets, 2);

		// Set data for instance buffer once
		mRenderer->VUpdateShaderInstanceBuffer(mStaticMeshShaderResource, mLevel.staticMeshWorldMatrices, ibStaticMeshSizes[0], 0);
		mRenderer->VUpdateShaderInstanceBuffer(mStaticMeshShaderResource, &mLevel.materialIDs[0], ibStaticMeshSizes[1], 1);

		// Constant buffer data
		void*	cStaticMeshData[] = { mCameraManager->GetCBufferPersp() };
		size_t	cbStaticMeshSizes[] = { sizeof(CBuffer::Camera) };

		mRenderer->VCreateShaderConstantBuffers(mStaticMeshShaderResource, cStaticMeshData, cbStaticMeshSizes, 1);

		// Textures
		std::vector<const char*> filenames;
		for (uint32_t i = 0; i < mLevel.textureNames.size(); i++)
		{
			filenames.push_back(mLevel.textureNames[i].c_str());
		}

		mRenderer->VCreateShaderTexture2DArray(mStaticMeshShaderResource, &filenames[0], mLevel.textureNames.size());
		mRenderer->VAddShaderLinearSamplerState(mStaticMeshShaderResource, SAMPLER_STATE_ADDRESS_WRAP);
	}

	// Characters
	{
		mRenderer->VCreateShaderResource(&mExplorerShaderResource, &mAllocator);

		void* cbExplorerData[] = { mCameraManager->GetCBufferPersp(), &mModel, mSkinnedMeshMatrices, nullptr };
		size_t cbExplorerSizes[] = { sizeof(CBuffer::Camera), sizeof(CBuffer::Model), sizeof(mat4f) *  MAX_SKELETON_JOINTS, sizeof(vec4f) };

		mRenderer->VCreateShaderConstantBuffers(mExplorerShaderResource, cbExplorerData, cbExplorerSizes, 4);

		const char* filenames[] = { 
			"Assets/Textures/BascMinionFull.png", 
			"Assets/Textures/flytraptxt.png", 
			"Assets/Textures/StaticMesh/Door.png", 
			"Assets/Textures/Sprinter_D_1024.png", 
			"Assets/Textures/Heal.png",
			"Assets/Textures/Trap.png",
			"Assets/Textures/Professor.png",
			"Assets/Textures/Trapper_D_1024.png"
		};
		
		mRenderer->VAddShaderTextures2D(mExplorerShaderResource, filenames, 8);
		mRenderer->VAddShaderLinearSamplerState(mExplorerShaderResource, SAMPLER_STATE_ADDRESS_WRAP);
	}

	// PVL
	{
		mRenderer->VCreateShaderResource(&mPLVShaderResource, &mAllocator);

		void* cbPVLData[] = { &mLightData, mCameraManager->GetOrigin().pCols, &mTime, &mPointLight };
		size_t cbPVLSizes[] = { sizeof(CBuffer::Light), sizeof(vec4f), sizeof(CBuffer::Effect), sizeof(CBuffer::PointLight) };

		mRenderer->VCreateShaderConstantBuffers(mPLVShaderResource, cbPVLData, cbPVLSizes, 4);
		mRenderer->VAddShaderLinearSamplerState(mPLVShaderResource, SAMPLER_STATE_ADDRESS_BORDER, const_cast<float*>(Colors::black.pCols));
		mRenderer->AddAdditiveBlendState(mPLVShaderResource);
	}

	// Sprites
	{
		mRenderer->VCreateShaderResource(&mSpritesShaderResource, &mAllocator);

		// SpriteSheets
		mRenderer->VCreateShaderTexture2DArray(mSpritesShaderResource, mSpriteManager->GetTextureNames(), mSpriteManager->GetTextureCount());
		mRenderer->VAddShaderLinearSamplerState(mSpritesShaderResource, SAMPLER_STATE_ADDRESS_WRAP);

		void*  cbSpritesData[] = { mCameraManager->GetCBufferPersp(), mSpriteManager->GetCBuffer() };
		size_t cbSpritesSizes[] = { sizeof(CBuffer::Camera), sizeof(CBuffer::SpriteSheet) * MAX_SPRITESHEETS };

		mRenderer->VCreateShaderConstantBuffers(mSpritesShaderResource, cbSpritesData, cbSpritesSizes, 2);

		// Instance buffer data
		void*	ibSpriteData[] = { mSpriteManager->GetSpriteInstanceBuffer(), mSpriteManager->GetGlyphInstanceBuffer() };
		size_t	ibSpriteSizes[] = { sizeof(GPU::Sprite) * MAX_SPRITES, sizeof(GPU::Glyph) * MAX_GLYPHS };
		size_t	ibSpriteStrides[] = { sizeof(GPU::Sprite), sizeof(GPU::Glyph) };
		size_t	ibSpriteOffsets[] = { 0, 0 };

		// Create the instance buffer
		mRenderer->VCreateDynamicShaderInstanceBuffers(mSpritesShaderResource, ibSpriteData, ibSpriteSizes, ibSpriteStrides, ibSpriteOffsets, 2);
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
		mRenderer->VCreateContextResourceTargets(mGridContext, 5, mGridData.mapTexWidth, mGridData.maxTexHeight);
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

void Level01::InitializeGameState()
{
	for (DominationPoint& dp : Factory<DominationPoint>())
	{
		if (dp.mTier == 0)
		{
			dp.mController->mActivationPredicate = nullptr;
		}
		else
		{
			dp.mController->mActivationPredicate = Level01::ActivationPredicate;
		}
	}
}

#pragma endregion

#pragma region Update

void Level01::VUpdate(double milliseconds)
{
	// Handle this first so we can destroy objects later.
	mCollisionManager->Update(milliseconds);

	// Handle this first so it can consume mouse events.
	mUIManager.Update(milliseconds);

	// TO DO: Possibly a Components Update method... if we move this code to application level.
	for (ExplorerController& ec : Factory<ExplorerController>())
	{
		ec.Update(milliseconds);
	}

	for (auto& mc : Factory<Minion>())
	{
		if (mNetworkManager->mMode == NetworkManager::SERVER)
		{
			mc.mController->Update(milliseconds);
		}

		if (mc.mShouldDestroy)
		{
			Factory<Minion>::Destroy(&mc);
		}
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
		

#ifdef _DEBUG
	if (mInput->GetKeyDown(KEYCODE_F3))
	{
		gDebugGrid = !gDebugGrid;
	}

	if (mInput->GetKeyDown(KEYCODE_F4))
	{
		gDebugColl = !gDebugColl;
	}

	if (mInput->GetKeyDown(KEYCODE_F5))
	{
		gDebugOrto = !gDebugOrto;
	}

	if (mInput->GetKeyDown(KEYCODE_F6))
	{
		gDebugGBuffer = !gDebugGBuffer;
	}

	if (mInput->GetKeyDown(KEYCODE_F7))
	{
		gDebugBVH = !gDebugBVH;
	}

	if (mInput->GetKeyDown(KEYCODE_F8))
	{
		gDebugBT = !gDebugBT;
	}
	//Do not use F9, already used else-where
	if (mInput->GetKeyDown(KEYCODE_F10))
	{
		SetRestart();
	}
#endif

	UpdateGameState(milliseconds); //Input

	mNetworkManager->Update();
}

void Level01::VFixedUpdate(double milliseconds)
{
	for (AnimationController& ac : Factory<AnimationController>())
	{
		ac.Update(milliseconds);
	}

	float seconds = static_cast<float>(milliseconds) / 1000.0f;
	mTime.delta += seconds;

	for (Heal& h : Factory<Heal>())
	{
		h.Update(seconds);
	}

	for (Explosion& e : Factory<Explosion>())
	{
		e.Update(seconds);
	}

	for (Trap& t : Factory<Trap>())
	{
		t.Update(seconds);
		if (t.mShouldDestroy)
		{
			Factory<Trap>::Destroy(&t);
		}
	}

	for (Lantern& l : Factory<Lantern>())
	{
		l.Update(seconds);
		if (l.mShouldDestroy)
		{
			Factory<Lantern>::Destroy(&l);
		}
	}

	for (StatusEffect& s : Factory<StatusEffect>())
	{
		if (!s.mIsActive)
		{
			continue;
		}

		s.Update(seconds);

		if (s.mShouldDestroy)
		{
			Factory<StatusEffect>::Destroy(&s);
		}
	}

	for (Transmogrify& t : Factory<Transmogrify>())
	{
		t.Update(seconds);
	}

	if (mAIManager->IsGridDirty()) ComputeGrid(); 
	mAIManager->Update();
}

void Level01::SetReady(int clientID)
{
	Packet p(READY);
	p.ClientID = clientID;
	p.AsBool = !mUIManager.GetReadyState(p.ClientID);

	mUIManager.SetReadyState(p.ClientID, p.AsBool);

	if (mNetworkManager->mMode == NetworkManager::Mode::CLIENT) {
		mNetworkManager->mClient.SendData(&p);
	}
	else if (mNetworkManager->mMode == NetworkManager::Mode::SERVER) {
		mNetworkManager->mServer.SendToAll(&p);
	}
}

void Level01::SetRestart()
{
	Packet p(RESTART);
	
	Restart();

	if (mNetworkManager->mMode == NetworkManager::Mode::CLIENT) {
		mNetworkManager->mClient.SendData(&p);
	}
	else if (mNetworkManager->mMode == NetworkManager::Mode::SERVER) {
		mNetworkManager->mServer.SendToAll(&p);
	}
}

void Level01::Restart()
{
	TRACE_LOG("Restarting");
	
	CLEAR_FACTORY(Minion);
	CLEAR_FACTORY(Lantern);
	CLEAR_FACTORY(Trap);
	CLEAR_FACTORY(StatusEffect);

	for (auto& lamp : Factory<Lamp>())
	{
		lamp.mStatus = LAMP_OFF;
	}

	for (auto& door : Factory<Door>())
	{
		if (!door.mColliderComponent->mIsActive) door.ToggleDoor();
	}

	for (auto& d : Factory<DominationPoint>())
	{
		d.mController->mProgress = 0;
		d.mController->isDominated = false;
	}

	SpawnPoint& sp = *(Factory<SpawnPoint>().begin());
	for (auto& e : Factory<Explorer>())
	{
		auto rndPos = sp.mTransform->GetPosition() + vec3f(Mathf::RandomRange(-4, 4), Mathf::RandomRange(-4, 4), 0);
		e.mTransform->SetPosition(rndPos);
		e.OnMove(&e, rndPos, e.mTransform->GetRotation());
		e.mHealth->SetHealth(9999);
	}

	for (auto& g : Factory<Ghost>())
	{
		g.TickMana(99999);
	}

	mGameState = GAME_STATE_INITIAL;
	mUIManager.SetReadyState(0, false);
	mUIManager.SetReadyState(1, false);
	mUIManager.SetReadyState(2, false);
	mUIManager.SetReadyState(3, false);
}

void Level01::UpdateGameState(double milliseconds)
{
	GameState currentState = mGameState;

	static DominationPoint* finalDP = nullptr;

	switch (currentState)
	{
	case GAME_STATE_INITIAL:
#ifdef _DEBUG
		if (mInput->GetKeyDown(KEYCODE_F9))
		{
			for (int i = 0; i < MAX_PLAYERS; i++) {
				SetReady(i);
			}
		}
#endif
		if (mInput->GetKeyDown(KEYCODE_SPACE))
		{
			SetReady(mNetworkManager->ID());
		}

		if (mUIManager.IsEveryoneReady()) {
			currentState = GAME_STATE_CAPTURE_0;
			mUIManager.BlockGame(false);
		}
		break;
	case GAME_STATE_CAPTURE_0:
	case GAME_STATE_CAPTURE_1:
	{
		bool checkDomPoints = IsExplorerAlive();
		if (checkDomPoints)
		{
			// Count dom points captured
			char dominationPointsCaptured = 0;
			for (DominationPoint& dp : Factory<DominationPoint>())
			{
				if (dp.mController->isDominated) { dominationPointsCaptured++; }
			}

			if (dominationPointsCaptured == 3)
			{
				// Final domination left
				currentState = GAME_STATE_CAPTURE_1;
			}

			if (dominationPointsCaptured == 4)
			{
				// Explorers Win
				currentState = GAME_STATE_FINAL_EXPLORERS_WIN;
			}
		}
		else
		{
			// Ghost Wins
			currentState = GAME_STATE_FINAL_GHOST_WIN;
		}
		
		break;	
	}
	case GAME_STATE_FINAL_GHOST_WIN:
	case GAME_STATE_FINAL_EXPLORERS_WIN:
		// Check for RESTART / EXIT.
		break;
	}

	mGameState = currentState;
}

bool Level01::IsExplorerAlive()
{

	int explorerCount = 0;
	for (Explorer& e : Factory<Explorer>())
	{
		explorerCount++;
		if (e.mHealth->GetHealth() > 0)
		{
			return true;
		}
	}

	if (explorerCount == 0)
	{
		return true;
	}

	return false;
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
	if (gDebugColl)
	RenderWallColliders(mExplorerShaderResource, mCameraManager, &mModel);
#endif
	
	RenderDoors();
	RenderExplorers();
	RenderMinions();
	RenderSpotLightVolumes();

#ifdef _DEBUG
	if (gDebugGBuffer)
	{
		RenderGBuffer(mGBufferContext);
	}
	else
	{
		RenderFullScreenQuad();
	}
#else
	RenderFullScreenQuad();
#endif
	mRenderer->GetDeviceContext()->PSSetShaderResources(0, 4, mNullSRV);

	RenderEffects();
	
	RenderWorldSpaceSprites();
	mUIManager.RenderPanel();
	if (mNetworkManager->mMode == NetworkManager::SERVER) mUIManager.RenderManaBar();
	mUIManager.RenderObjectives(mGameState, mNetworkManager->mMode == NetworkManager::SERVER);
	
	switch (mGameState)
	{
	case GAME_STATE_FINAL_GHOST_WIN:
		mUIManager.RenderEndScreen(true, [this]() {SetRestart(); return true; });
		break;
	case GAME_STATE_FINAL_EXPLORERS_WIN:
		mUIManager.RenderEndScreen(false, [this]() {SetRestart(); return true; });
		break;
	case GAME_STATE_INITIAL:
		mUIManager.RenderReadyScreen(mNetworkManager->ID());
		break;
	default:
		break;
	}

	RenderIMGUI();
	RenderSprites();


#ifdef _DEBUG
	if (gDebugGrid) RenderGrid();
	RENDER_TRACE();
#endif

	mRenderer->VSwapBuffers();
}

void Level01::RenderShadowMaps()
{
	mRenderer->GetDeviceContext()->RSSetState(nullptr);

	mRenderer->SetViewport(0.0f, 0.0f, static_cast<float>(SHADOW_MAP_SIZE), static_cast<float>(SHADOW_MAP_SIZE), 0.0f, 1.0f);
	mRenderer->VSetInputLayout(mApplication->mVSDefSingleMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefSingleMaterial);
	mRenderer->VSetPixelShader(mApplication->mPSFwdDistanceMaterial);

	// Identity here just to be compatible with shader.
	mLightPVM.view = mat4f(1.0f);	

	uint32_t lampIndex = 0;
	for (Lamp& lamp : Factory<Lamp>())
	{
		float color[4] = { 0.0f, 0.0f, 0.0, 1.0f };

		mPointLight.color = lamp.mLightColor;
		mPointLight.position = lamp.mTransform->GetPosition();
		mPointLight.radius = lamp.mLightRadius;
		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mPointLight, 3);
		mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 3, 0);

		for (uint32_t faceIndex = 0; faceIndex < 6; faceIndex++)
		{
			uint32_t projIndex = (lampIndex * 6) + faceIndex;
			
			mRenderer->VSetRenderContextTargetWithDepth(mShadowContext, projIndex, 0);
			mRenderer->VClearContextTarget(mShadowContext, projIndex, color);
			mRenderer->VClearDepthStencil(mShadowContext, 0, 1.0f, 0);
		
			// Set view projection matrix for light frustum
			mLightPVM.projection = mLevel.lampVPTMatrices[projIndex].transpose();

			// Create frustum object for culling.
			Rig3D::Frustum frustum;
			Rig3D::ExtractNormalizedFrustumLH(&frustum, mLevel.lampVPTMatrices[projIndex]);

			for (int staticMeshIndex = 0; staticMeshIndex < STATIC_MESH_MODEL_COUNT; staticMeshIndex++)
			{
				if (staticMeshIndex == STATIC_MESH_WALL_LANTERN) { continue; }

				// Get Objects for a given mesh
				std::vector<BaseSceneObject*>* pBaseSceneObjects = mModelManager->RequestAllUsingModel(kStaticMeshModelNames[staticMeshIndex]);

				// Storage for indices we will draw
				std::vector<uint32_t> indices;
				indices.reserve(pBaseSceneObjects->size());

				// Cull meshes given the current lamp frustum
				CullOBBSceneObjects<StaticMesh>(frustum, *pBaseSceneObjects, indices);

				// Bind Mesh
				IMesh* mesh = mModelManager->GetModel(kStaticMeshModelNames[staticMeshIndex])->mMesh;
				mRenderer->VBindMesh(mesh);

				uint32_t indexCount = mesh->GetIndexCount();

				// Render 
				for (uint32_t filterIndex : indices)
				{
					mLightPVM.world = (*pBaseSceneObjects)[filterIndex]->mTransform->GetWorldMatrix().transpose();
					mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM, 0);
					mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM.world, 1);

					mRenderer->VSetVertexShaderConstantBuffers(mExplorerShaderResource);
					mRenderer->VDrawIndexed(0, indexCount);
				}
			}

			std::vector<BaseSceneObject*>* pDoors = mModelManager->RequestAllUsingModel(kDoorModelName);

			// Storage for indices we will draw
			std::vector<uint32_t> indices;
			indices.reserve(pDoors->size());

			// Cull meshes given the current lamp frustum
			CullOBBSceneObjects<Door>(frustum, *pDoors, indices);

			// Bind Mesh
			IMesh* mesh = mModelManager->GetModel(kDoorModelName)->mMesh;
			mRenderer->VBindMesh(mesh);

			uint32_t indexCount = mesh->GetIndexCount();

			// Render 
			for (uint32_t filterIndex : indices)
			{
				mLightPVM.world = (*pDoors)[filterIndex]->mTransform->GetWorldMatrix().transpose();
				mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM, 0);
				mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM.world, 1);

				mRenderer->VSetVertexShaderConstantBuffers(mExplorerShaderResource);
				mRenderer->VDrawIndexed(0, indexCount);
			}
		}

		lampIndex++;
	}
}

void Level01::RenderStaticMeshes()
{
	mRenderer->SetViewport();
	mRenderer->VSetRenderContextTargetsWithDepth(mGBufferContext, 0);

	mRenderer->VClearContextTarget(mGBufferContext, 0, Colors::magentaAlpha.pCols);	// Position
	mRenderer->VClearContextTarget(mGBufferContext, 1, Colors::magentaAlpha.pCols);	// Normal
	mRenderer->VClearContextTarget(mGBufferContext, 2, Colors::magentaAlpha.pCols);	// Color
	mRenderer->VClearDepthStencil(mGBufferContext, 0, 1.0f, 0);						// Depth

	mRenderer->VSetInputLayout(mApplication->mVSDefInstancedMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefInstancedMaterial);
	mRenderer->VSetPixelShader(mApplication->mPSDefInstancedMaterial);

	mRenderer->VUpdateShaderConstantBuffer(mStaticMeshShaderResource, mCameraManager->GetCBufferPersp(), 0);

	mRenderer->VSetVertexShaderConstantBuffer(mStaticMeshShaderResource, 0, 0);
	mRenderer->VSetVertexShaderInstanceBuffer(mStaticMeshShaderResource, 0, 1);
	mRenderer->VSetVertexShaderInstanceBuffer(mStaticMeshShaderResource, 1, 2);

	mRenderer->VSetPixelShaderResourceView(mStaticMeshShaderResource, 0, 0);
	mRenderer->VSetPixelShaderSamplerStates(mStaticMeshShaderResource);

	int instanceCount = 0;
	
	// NOTE: This will break if we don't create meshes by model.
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
}

void Level01::RenderDoors()
{
	mRenderer->VSetInputLayout(mApplication->mVSDefSingleMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSDefSingleMaterial);
	mRenderer->VSetPixelShader(mApplication->mPSDefMaterial);

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 3, 2);
	mRenderer->VSetPixelShaderResourceView(mExplorerShaderResource, 2, 0);

	ModelCluster* model = nullptr;

	for (Door& d : Factory<Door>())
	{
		if (!model)
		{
			model = d.mModel;
			mRenderer->VBindMesh(model->mMesh);
		}

		mModel.world = d.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VDrawIndexed(0, model->mMesh->GetIndexCount());
	}

	// Putting Domination Points here for now.
	model = mModelManager->GetModel(kStaticMeshModelNames[STATIC_MESH_MODEL_GENERATOR]);
	mRenderer->VBindMesh(model->mMesh);

	vec4f gray = { 0.75f, 0.75f, 0.75f, 1.0f };

	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &gray, 3);

	for (DominationPoint& dp : Factory<DominationPoint>())
	{
		mModel.world = dp.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VDrawIndexed(0, model->mMesh->GetIndexCount());
	}

	model = mModelManager->GetModel(kLanternModelName);
	mRenderer->VBindMesh(model->mMesh);

	for (Lantern& l : Factory<Lantern>())
	{
		mModel.world = l.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VDrawIndexed(0, model->mMesh->GetIndexCount());
	}
}

void Level01::RenderEffects()
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 0.7f };

	// Use the default context with depth buffer from the g-buffer pass.
	
	mRenderer->VSetContextTargetWithDepth(mGBufferContext, 0);

	mRenderer->VSetInputLayout(mApplication->mVSFwdSingleMaterial);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSingleMaterial);
	mRenderer->VSetPixelShader(mApplication->mPSFwdSingleMaterial);

	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);
	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 2, 0);
	mRenderer->VBindMesh(mSphereMesh);
		
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);
	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);

	mTime.color = { 0.1f, 0.1f, 0.1f, 1.0f };
	mTime.cameraPosition = mCameraManager->GetOrigin();
	mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mTime, 2);
	mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 2, 0);

	//mTime.rate = 1.0f;
	//mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mTime, 2);
	//mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 2, 0);

	for (Heal& heal : Factory<Heal>())
	{
		mModel.world = heal.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VDrawIndexed(0, mSphereMesh->GetIndexCount());
	}

	for (Explosion& explosion : Factory<Explosion>())
	{
		mModel.world = explosion.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VDrawIndexed(0, mSphereMesh->GetIndexCount());
	}


	mTime.color = { 1.0f, 0.1f, 1.0f, 1.0f };

	for (Trap& trap : Factory<Trap>())
	{
		if (!trap.mEffect->mIsActive)
		{
			continue;
		}

		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mTime, 2);
		mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 2, 0);

		mModel.world = trap.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VDrawIndexed(0, mSphereMesh->GetIndexCount());
	}

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, color, 0xffffffff);
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
	mRenderer->VSetPixelShaderSamplerStates(mExplorerShaderResource);

	uint8_t materialIDs[4] = { 6, 3, 7, 0 };

	for (Explorer& e : Factory<Explorer>())
	{
		mModel.world = e.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		e.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);
		
		uint8_t materialIndex = (e.mIsTransmogrified) ? 3 : e.GetExplorerType() - 1;

		mRenderer->VSetPixelShaderResourceView(mExplorerShaderResource, materialIDs[materialIndex], 0);

		mRenderer->VBindMesh(e.mModel->mMesh);
		mRenderer->VDrawIndexed(0, e.mModel->mMesh->GetIndexCount());
	}

	IMesh* pTrapMesh = mModelManager->GetModel(kTrapModelName)->mMesh;
	mRenderer->VBindMesh(pTrapMesh);
	mRenderer->VSetPixelShaderResourceView(mExplorerShaderResource, 5, 0);

	for (Trap& trap: Factory<Trap>())
	{
		mModel.world = trap.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		trap.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);

		mRenderer->VDrawIndexed(0, pTrapMesh->GetIndexCount());
	}
}

void Level01::RenderSpotLightVolumes()
{
	// Cull light volumes first
	Frustum frustum;
	mat4f projView = (mCameraManager->GetCBufferPersp()->projection * mCameraManager->GetCBufferPersp()->view).transpose();
	ExtractNormalizedFrustumLH(&frustum, projView);

	std::vector<std::pair<Lamp*, uint32_t>> lampPairs;
	CullLamps(frustum, &lampPairs);

	std::vector<std::pair<Lantern*, uint32_t>> lanternPairs;
	CullLanterns(frustum, &lanternPairs);

	//mRenderer->VClearStencil(mGBufferContext, 0, 0);

	//// Stencil pass
	//mRenderer->GetDeviceContext()->RSSetState(mLightingWriteRS);
	//mRenderer->GetDeviceContext()->OMSetDepthStencilState(mLightingWriteDSS, 0);

	//mRenderer->VSetInputLayout(mApplication->mVSFwdSpotLightVolume);
	//mRenderer->VSetVertexShader(mApplication->mVSFwdSpotLightVolume);
	//mRenderer->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);

	mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, mCameraManager->GetOrigin().pCols, 1);
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferPersp(), 0);

	mRenderer->VBindMesh(mGeodesicSphereMesh);

	//for (std::pair<Lamp*, uint32_t> lampPair : lampPairs)
	//{
	//	mModel.world = mLevel.lampWorldMatrices[lampPair.second];
	//	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
	//	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
	//	mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);
	//	mRenderer->VDrawIndexed(0, mGeodesicSphereMesh->GetIndexCount());
	//}

	// Albedo Pass
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);
	mRenderer->GetDeviceContext()->RSSetState(mLightingReadRS);
	//mRenderer->GetDeviceContext()->OMSetDepthStencilState(mLightingReadDSS, 0);

	mRenderer->VSetRenderContextTarget(mGBufferContext, 3);
//	mRenderer->VSetRenderContextTargetWithDepth(mGBufferContext, 3, 0);
	mRenderer->VClearContextTarget(mGBufferContext, 3, Colors::black.pCols);	// Albedo

	mRenderer->VSetInputLayout(mApplication->mVSFwdSpotLightVolume);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSpotLightVolume);
	mRenderer->VSetPixelShader(mApplication->mPSFwdPointLightVolume);

	for (std::pair<Lamp*, uint32_t> lampPair : lampPairs)
	{
		mModel.world = mLevel.lampWorldMatrices[lampPair.second];
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		// Set Light data
		mLightData.viewProjection = (mLevel.lampVPTMatrices[lampPair.second]).transpose();
		mLightData.color = lampPair.first->mLightColor;
		if (lampPair.first->mStatus == LAMP_DIMMED) mLightData.color *= 0.4f;
		mLightData.direction = lampPair.first->mLightDirection;
		mLightData.range = lampPair.first->mLightRadius;
		mLightData.cosAngle = cos(lampPair.first->mLightAngle);

		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mLightData, 0);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 0, 0);

		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 0, 0);					// Position
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 1, 1);					// Normal
		mRenderer->VSetPixelShaderResourceView(mShadowContext, lampPair.second, 2);		// Shadow
		mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);					// Border

		mRenderer->VDrawIndexed(0, mGeodesicSphereMesh->GetIndexCount());
	}

	mRenderer->VSetPixelShader(mApplication->mPSFwdPointLightVolumeNS);

	for (std::pair<Lantern*, uint32_t> lanternPair : lanternPairs)
	{
		mModel.world = (mat4f::scale(lanternPair.first->mColliderComponent->mCollider.radius) * mat4f::translate(lanternPair.first->GetLightPosition())).transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		// Set Light data
		mLightData.color = lanternPair.first->mLightColor;
		mLightData.range = lanternPair.first->mColliderComponent->mCollider.radius;

		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mLightData, 0);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 0, 0);

		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 0, 0);					// Position
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 1, 1);					// Normal
		mRenderer->VDrawIndexed(0, mGeodesicSphereMesh->GetIndexCount());
	}

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, color, 0xffffffff);
	mRenderer->GetDeviceContext()->RSSetState(nullptr);
	//mRenderer->GetDeviceContext()->OMSetDepthStencilState(nullptr, 0);
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

	for (MinionController& mc : Factory<ImpController>())
	{
		Minion& m = *static_cast<Minion*>(mc.mSceneObject);

		mModel.world = m.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		m.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);

		mRenderer->VBindMesh(m.mModel->mMesh);
		mRenderer->VDrawIndexed(0, m.mModel->mMesh->GetIndexCount());
	}

	for (AbominationController& mc : Factory<AbominationController>())
	{
		Minion& m = *static_cast<Minion*>(mc.mSceneObject);

		mModel.world = m.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		m.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);

		mRenderer->VBindMesh(m.mModel->mMesh);
		mRenderer->VDrawIndexed(0, m.mModel->mMesh->GetIndexCount());
	}

	mRenderer->VSetPixelShaderResourceView(mExplorerShaderResource, 1, 0);

	for (MinionController& mc : Factory<FlyTrapController>())
	{
		Minion& ft = *static_cast<Minion*>(mc.mSceneObject);

		mModel.world = ft.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

		ft.mAnimationController->mSkeletalHierarchy.CalculateSkinningMatrices(mSkinnedMeshMatrices);
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mSkinnedMeshMatrices, 2);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 2, 2);

		mRenderer->VBindMesh(ft.mModel->mMesh);
		mRenderer->VDrawIndexed(0, ft.mModel->mMesh->GetIndexCount());
	}
}

void Level01::RenderWorldSpaceSprites()
{
	bool isGhost = mNetworkManager->ID() == 0;

	for (auto& d : Factory<DominationPoint>())
	{
		if (d.mTier == 1 && mGameState < GAME_STATE_CAPTURE_1) continue;
		auto screenPos = mCameraManager->World2Screen(d.mTransform->GetPosition());
		mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, 6, screenPos, isGhost ? vec2f(70, 70) : vec2f(300, 300), vec4f(1, 1, 1, .5f));
		mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, 6, screenPos, isGhost ? vec2f(70, 70) : vec2f(300, 300), vec4f(1, 0, 0, .5f), vec2f(1, 1), 2 * PI * d.mController->mProgress);
	}

	for (Health& h : Factory<Health>())
	{
		auto screenPos = mCameraManager->World2Screen(h.mSceneObject->mTransform->GetPosition()) + vec2f(0, isGhost ? -30.0f : -90.0f);
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 1, screenPos, isGhost ? vec2f(75, 11) : vec2f(90, 12), vec4f(1, 1, 1, 1), vec2f(h.GetHealthPerc(), 1));
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 0, screenPos, isGhost ? vec2f(75, 11) : vec2f(90, 12));
	}

	for (StatusEffect& s : Factory<StatusEffect>())
	{
		for (auto &m : s.mMinions) {
			auto screenPos = mCameraManager->World2Screen(m.first->mTransform->GetPosition());
			mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, isGhost ? 10.0f : 14.0f, screenPos, vec4f(1, 1, 1, 1), ALIGN_CENTER, "%s", s.mFlavorText);
		}
		for (auto &m : s.mExplorers) {
			auto screenPos = mCameraManager->World2Screen(m.first->mTransform->GetPosition());
			mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, isGhost ? 10.0f : 14.0f, screenPos, vec4f(1, 1, 1, 1), ALIGN_CENTER, "%s", s.mFlavorText);
		}
	}
}

void Level01::RenderSprites() {
	mRenderer->VSetContextTarget();

	mRenderer->VSetInputLayout(mApplication->mVSFwdSprites);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSprites);
	mRenderer->VSetPixelShader(mApplication->mPSFwd2DTexture);

	mRenderer->VUpdateShaderConstantBuffer(mSpritesShaderResource, mCameraManager->GetCBufferScreenOrto(), 0);
	mRenderer->VSetVertexShaderConstantBuffers(mSpritesShaderResource);

	mRenderer->VUpdateShaderInstanceBuffer(mSpritesShaderResource, mSpriteManager->GetSpriteInstanceBuffer(), sizeof(GPU::Sprite) * mSpriteManager->GetSpriteInstanceBufferCount(), 0);
	mRenderer->VUpdateShaderInstanceBuffer(mSpritesShaderResource, mSpriteManager->GetGlyphInstanceBuffer(), sizeof(GPU::Glyph) * mSpriteManager->GetGlyphInstanceBufferCount(), 1);

	mRenderer->VSetPixelShaderResourceView(mSpritesShaderResource, 0, 0);
	mRenderer->VSetPixelShaderSamplerStates(mSpritesShaderResource);

	mRenderer->VBindMesh(mNDSQuadMesh);
	mRenderer->VSetVertexShaderInstanceBuffer(mSpritesShaderResource, 0, 1);
	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mNDSQuadMesh->GetIndexCount(), mSpriteManager->GetSpriteInstanceBufferCount(), 0, 0, 0);

	mRenderer->VSetInputLayout(mApplication->mVSFwdSpriteGlyphs);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSpriteGlyphs);

	mRenderer->VSetVertexShaderInstanceBuffer(mSpritesShaderResource, 1, 1);
	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mNDSQuadMesh->GetIndexCount(), mSpriteManager->GetGlyphInstanceBufferCount(), 0, 0, 0);

	mSpriteManager->EndFrame();
}

void Level01::RenderGrid()
{
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
}

#pragma endregion

void Level01::ComputeGrid()
{
	if (mNetworkManager->mMode != NetworkManager::SERVER) return;

	// Prepare visual data (aka draw a bunch of stuff again)
	mRenderer->VSetRenderContextTargetsWithDepth(mGridContext, 0);
	
	// Reset state
	mRenderer->GetDeviceContext()->RSSetState(nullptr);
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mRenderer->SetViewport(0.0f, 0.0f, static_cast<float>(mLevel.extents.x*GRID_MAP_SCALE), static_cast<float>(mLevel.extents.y*GRID_MAP_SCALE), 0.0f, 1.0f);
	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, Colors::whiteAlpha.pCols, 0xffffffff);

	mRenderer->VClearContextTarget(mGridContext, 0, Colors::magentaAlpha.pCols);	// Position
	mRenderer->VClearContextTarget(mGridContext, 1, Colors::magentaAlpha.pCols);	// Normal
	mRenderer->VClearContextTarget(mGridContext, 2, Colors::magentaAlpha.pCols);	// Color
	mRenderer->VClearDepthStencil(mGridContext, 0, 1.0f, 0);						// Depth

	//Static Meshes
	mRenderer->VSetInputLayout(mApplication->mVSDefInstancedColor);
	mRenderer->VSetVertexShader(mApplication->mVSDefInstancedColor);
	mRenderer->VSetPixelShader(mApplication->mPSDefColor);

	CBuffer::ObjectType obj;
	obj.color = vec4f(1, 1, 1, 1);

	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, mCameraManager->GetCBufferFullLevelOrto(), 0);
	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, &obj, 2);

	mRenderer->VSetVertexShaderConstantBuffer(mGridShaderResource, 0, 0);
	mRenderer->VSetVertexShaderConstantBuffer(mGridShaderResource, 2, 1);

	mRenderer->VSetVertexShaderInstanceBuffer(mStaticMeshShaderResource, 0, 1);
	
	//First no floors/stairs
	int instanceCount = 0;
	for (Factory<StaticMesh>::iterator it = Factory<StaticMesh>().begin(); it != Factory<StaticMesh>().end();)
	{
		auto& staticMesh = *it;
		auto modelCluster = staticMesh.mModel;
		auto numElements = modelCluster->ShareCount();

		if (modelCluster->mName != kStaticMeshModelNames[STATIC_MESH_MODEL_FLOOR] &&
			modelCluster->mName != kStaticMeshModelNames[STATIC_MESH_MODEL_CURVED_STAIRS] &&
			modelCluster->mName != kStaticMeshModelNames[STATIC_MESH_MODEL_CURVED_STAIRS_LEFT] &&
			modelCluster->mName != kStaticMeshModelNames[STATIC_MESH_STAIR_FULL] &&
			modelCluster->mName != kStaticMeshModelNames[STATIC_MESH_STAIR_HALF]) {
			mRenderer->VBindMesh(modelCluster->mMesh);
			mRenderer->GetDeviceContext()->DrawIndexedInstanced(modelCluster->mMesh->GetIndexCount(), numElements, 0, 0, instanceCount);
		}
		instanceCount += numElements;

		for (auto i = 0; i < numElements; i++) ++it;
	}

	mRenderer->VCopySubresource(mGridContext, 4, 2);

	//Now only the floors/stairs
	instanceCount = 0;
	for (Factory<StaticMesh>::iterator it = Factory<StaticMesh>().begin(); it != Factory<StaticMesh>().end();)
	{
		auto& staticMesh = *it;
		auto modelCluster = staticMesh.mModel;
		auto numElements = modelCluster->ShareCount();

		if (modelCluster->mName == kStaticMeshModelNames[STATIC_MESH_MODEL_FLOOR] &&
			modelCluster->mName == kStaticMeshModelNames[STATIC_MESH_MODEL_CURVED_STAIRS] &&
			modelCluster->mName == kStaticMeshModelNames[STATIC_MESH_MODEL_CURVED_STAIRS_LEFT] &&
			modelCluster->mName == kStaticMeshModelNames[STATIC_MESH_STAIR_FULL] &&
			modelCluster->mName == kStaticMeshModelNames[STATIC_MESH_STAIR_HALF]) {
			mRenderer->VBindMesh(modelCluster->mMesh);
			mRenderer->GetDeviceContext()->DrawIndexedInstanced(modelCluster->mMesh->GetIndexCount(), numElements, 0, 0, instanceCount);
			break; // Early exit;
		}
		instanceCount += numElements;

		for (auto i = 0; i < numElements; i++) ++it;
	}

	// Lights
	mRenderer->VSetRenderContextTarget(mGridContext, 3);
	mRenderer->VClearContextTarget(mGridContext, 3, Colors::black.pCols);	// Albedo

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);

	mRenderer->VSetInputLayout(mApplication->mVSFwdSpotLightVolume);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSpotLightVolume);
	mRenderer->VSetPixelShader(mApplication->mPSFwdPointLightVolume);

	mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, mCameraManager->GetOrigin().pCols, 1);
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferFullLevelOrto(), 0);

	mRenderer->VBindMesh(mGeodesicSphereMesh);

	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		if (l.mStatus == LAMP_ON) {
			mModel.world = mLevel.lampWorldMatrices[i];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

			// Set Light data
			mLightData.viewProjection = (mLevel.lampVPTMatrices[i]).transpose();
			mLightData.color = l.mLightColor;
			mLightData.direction = l.mLightDirection;
			mLightData.range = l.mLightRadius;
			mLightData.cosAngle = cos(l.mLightAngle);

			mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mLightData, 0);

			mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
			mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);

			mRenderer->VSetPixelShaderConstantBuffers(mPLVShaderResource);
			mRenderer->VSetPixelShaderConstantBuffer(mPLVShaderResource, 0, 0);

			mRenderer->VSetPixelShaderResourceView(mGridContext, 0, 0);		// Position
			mRenderer->VSetPixelShaderResourceView(mGridContext, 1, 1);		// Normal
			mRenderer->VSetPixelShaderResourceView(mShadowContext, i, 2);		// Shadow
			mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);		// Border

			mRenderer->VDrawIndexed(0, mGeodesicSphereMesh->GetIndexCount());
		}
		i++;
	}

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, color, 0xffffffff);
	
	// Open doors
	mRenderer->VSetRenderContextTarget(mGridContext, 4);
	mRenderer->VSetInputLayout(mApplication->mVSFwdSingleColor);
	mRenderer->VSetVertexShader(mApplication->mVSFwdSingleColor);
	mRenderer->VSetPixelShader(mApplication->mPSFwdColor);

	obj.color = Colors::cyanAlpha;
	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, &obj, 2);
	mRenderer->VSetVertexShaderConstantBuffer(mGridShaderResource, 2, 2);
	mRenderer->VBindMesh(mCubeMesh);

	for each (Door& d in Factory<Door>())
	{
		if (d.mColliderComponent->mIsActive) continue; //Door is closed
			
		mModel.world = (mat4f::scale(d.mTrigger->mCollider.halfSize * 2.0f) * mat4f::translate(d.mTrigger->mCollider.origin)).transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 1, 1);
		mRenderer->VDrawIndexed(0, mCubeMesh->GetIndexCount());
	}
	
	mRenderer->VSetContextTarget();

	// Reset current grid to -10 / 0 on player
	mAIManager->ResetGridData();

	//Pass 1
	mRenderer->VSetComputeShader(mApplication->mCSGridPass1);

	mRenderer->VUpdateShaderConstantBuffer(mGridShaderResource, mCameraManager->GetCBufferFullLevelOrto(), 0);
	mRenderer->VSetComputeShaderConstantBuffers(mGridShaderResource);

	mDeviceContext->UpdateSubresource(mFullSrcData, 0, NULL, mAIManager->mGrid.pList, 0, 0);
	mDeviceContext->CSSetShaderResources(0, 1, &mFullSrcDataSRV);			//Full grid data
	mRenderer->VSetComputeShaderResourceView(mGridContext, 3, 1);		//Shadow Map
	mRenderer->VSetComputeShaderResourceView(mGridContext, 4, 2);		//Walls + door Map
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

	//Copy previous results to a CPU friendly buffer
	mDeviceContext->CopyResource(mOutputDataCPURead, mOutputData);

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

	mAIManager->SetGridDirty(false);
}

void Level01::VShutdown()
{
	CLEAR_FACTORY(Ghost)
	CLEAR_FACTORY(Explorer)
	CLEAR_FACTORY(Minion)
	CLEAR_FACTORY(StaticMesh)
	CLEAR_FACTORY(StaticCollider)
	CLEAR_FACTORY(SpawnPoint)
	CLEAR_FACTORY(DominationPoint)
	CLEAR_FACTORY(Lamp)
	CLEAR_FACTORY(Lantern)
	CLEAR_FACTORY(Region)
	CLEAR_FACTORY(Door)
}

bool Level01::ActivationPredicate(class Explorer* explorer)
{
	for (DominationPoint& dp : Factory<DominationPoint>())
	{
		if (dp.mTier == 1) { continue; }
		
		if (dp.mController->isDominated == false)
		{
			return false;
		}
	}

	return true;
}