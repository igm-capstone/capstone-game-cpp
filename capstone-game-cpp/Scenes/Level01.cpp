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
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Heal.h>
#include <SceneObjects/Trap.h>
#include <SceneObjects/StatusEffect.h>
#include <SceneObjects/DominationPoint.h>
#include <Components/DominationPointController.h>
#include <Components/ImpController.h>
#include <Components/FlyTrapController.h>

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

extern bool gDebugExplorer;

Level01::Level01() :
	mCubeMesh(nullptr),
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
	mNDSQuadMesh->~IMesh();

	for (Lamp& l : Factory<Lamp>())
	{
		l.mConeMesh->~IMesh();
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

	mSpriteManager->Initialize(mNDSQuadMesh, mSpritesShaderResource);

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

	//mModelManager->LoadModel<GPU::SkinnedVertex>(kSprinterModelName);
	mModelManager->LoadModel<GPU::SkinnedVertex>(kMinionAnimModelName);
	mModelManager->LoadModel<GPU::SkinnedVertex>(kPlantModelName);

	mModelManager->LoadModel<GPU::Vertex3>(kDoorModelName);

	//mLevel = Resource::LoadLevel("Assets/Level02.json", mAllocator);
	mLevel = Resource::LoadLevel("Assets/Level02_Test.json", mAllocator);


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

	// Cube
	Rig3D::Geometry::Cube(vertices, indices, 2);

	meshLibrary.NewMesh(&mCubeMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mCubeMesh, &vertices[0], sizeof(GPU::Vertex3) * vertices.size(), sizeof(GPU::Vertex3));
	mRenderer->VSetMeshIndexBuffer(mCubeMesh, &indices[0], indices.size());

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

		const char* filenames[] = { "Assets/Textures/BascMinionFull.png", "Assets/Textures/flytraptxt.png", "Assets/Textures/StaticMesh/Door.png" };
		mRenderer->VAddShaderTextures2D(mExplorerShaderResource, filenames, 3);
		mRenderer->VAddShaderLinearSamplerState(mExplorerShaderResource, SAMPLER_STATE_ADDRESS_WRAP);
	}

	// PVL
	{
		mRenderer->VCreateShaderResource(&mPLVShaderResource, &mAllocator);

		void* cbPVLData[] = { &mLightData, mCameraManager->GetOrigin().pCols };
		size_t cbPVLSizes[] = { sizeof(CBuffer::Light), sizeof(vec4f) };

		mRenderer->VCreateShaderConstantBuffers(mPLVShaderResource, cbPVLData, cbPVLSizes, 2);
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

	// TO DO: Possibly a Components Update method... if we move this code to application level.
	for (ExplorerController& ec : Factory<ExplorerController>())
	{
		ec.Update(milliseconds);
	}

	for (auto& mc : Factory<Minion>())
	{
		mc.mController->Update(milliseconds);

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

	for (AnimationController& ac : Factory<AnimationController>())
	{
		ac.Update(milliseconds);
	}

	float seconds = static_cast<float>(milliseconds) / 1000.0f;
	for (Heal& h : Factory<Heal>())
	{
		h.mDuration -= seconds;
		if (h.mDuration <= 0.0f)
		{
			Factory<Heal>::Destroy(&h);
		}
	}

	for (Trap& t: Factory<Trap>())
	{
		t.mDuration -= seconds;
		if (t.mDuration <= 0.0f)
		{
			Factory<Trap>::Destroy(&t);
		}
	}

	for (StatusEffect& s : Factory<StatusEffect>())
	{
		s.mOnUpdateCallback(&s, seconds);
		s.mDuration -= seconds;
		if (s.mDuration <= 0.0f)
		{
			Factory<StatusEffect>::Destroy(&s);
		}
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
#endif

}

void Level01::VFixedUpdate(double milliseconds)
{
	if (mAIManager->IsGridDirty()) ComputeGrid(); 
	mAIManager->Update();

	mNetworkManager->Update();

	UpdateGameState(milliseconds);
}

void Level01::UpdateGameState(double milliseconds)
{
	GameState currentState = mGameState;

	static DominationPoint* finalDP = nullptr;

	switch (currentState)
	{
	case GAME_STATE_INITIAL:
		// Check for ready status
		currentState = GAME_STATE_CAPTURE_0;
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

	mSpriteManager->NewFrame();
	RenderHealthBars();
	mSkillBar.RenderPanel();
	if (mNetworkManager->mMode == NetworkManager::SERVER) mSkillBar.RenderManaBar();
	mSkillBar.RenderObjectives(mGameState, mNetworkManager->mMode == NetworkManager::SERVER);
	if (mGameState == GAME_STATE_FINAL_GHOST_WIN) mSkillBar.RenderEndScreen(true);
	if (mGameState == GAME_STATE_FINAL_EXPLORERS_WIN) mSkillBar.RenderEndScreen(false);

	RenderIMGUI(); 
	RenderSprites();

	mRenderer->GetDeviceContext()->PSSetShaderResources(0, 4, mNullSRV);

#ifdef _DEBUG
	if (gDebugGrid) RenderGrid();
	RENDER_TRACE();
#endif

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

	uint32_t lampIndex = 0;
	for (Lamp& lamp : Factory<Lamp>())
	{
		mRenderer->VSetRenderContextDepthTarget(mShadowContext, lampIndex);
		mRenderer->VClearDepthStencil(mShadowContext, lampIndex, 1.0f, 0);

		// Set view projection matrix for light frustum
		mLightPVM.projection = mLevel.lampVPTMatrices[lampIndex].transpose();
		
		// Create frustum object for culling.
		Rig3D::Frustum frustum;
		Rig3D::ExtractNormalizedFrustumLH(&frustum, mLevel.lampVPTMatrices[lampIndex]);

		for (int staticMeshIndex = 0; staticMeshIndex < STATIC_MESH_MODEL_COUNT; staticMeshIndex++)
		{
			if (staticMeshIndex == STATIC_MESH_WALL_LANTERN) { continue; }

			// Get Objects for a given mesh
			std::vector<BaseSceneObject*>* pBaseSceneObjects = mModelManager->RequestAllUsingModel(kStaticMeshModelNames[staticMeshIndex]);
			
			// Storage for indices we will draw
			std::vector<uint32_t> indices;
			indices.reserve(pBaseSceneObjects->size());

			// Cull meshes given the current lamp frustum
			CullAABBSceneObjects<StaticMesh>(frustum, *pBaseSceneObjects, indices);

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
	model = mModelManager->GetModel(kStaticMeshModelNames[STATIC_MESH_MODEL_DOM_POINT]);
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
		if (l.mStatus != LAMP_OFF) {
			mModel.world = mLevel.lampWorldMatrices[i];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mModel, 1);

			// Set Light data
			mLightData.viewProjection = (mLevel.lampVPTMatrices[i]).transpose();
			mLightData.color = l.mLightColor;
			if (l.mStatus == LAMP_DIMMED) mLightData.color *= 0.4f;
			mLightData.direction = l.mLightDirection;
			mLightData.range = l.mLightRadius;
			mLightData.cosAngle = cos(l.mLightAngle);

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
		}
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

void Level01::RenderHealthBars()
{
	UINT sCount = 0;
	for (Health& h : Factory<Health>())
	{
		auto screenPos = mCameraManager->World2Screen(h.mSceneObject->mTransform->GetPosition()) + vec2f(0, -32);
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 1, screenPos, vec2f(90, 12), vec4f(1,1,1,1), vec2f(h.GetHealthPerc(), 1));
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 0, screenPos, vec2f(90, 12));
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
	
	//First no floors
	int instanceCount = 0;
	for (Factory<StaticMesh>::iterator it = Factory<StaticMesh>().begin(); it != Factory<StaticMesh>().end();)
	{
		auto& staticMesh = *it;
		auto modelCluster = staticMesh.mModel;
		auto numElements = modelCluster->ShareCount();

		if (modelCluster->mName != kStaticMeshModelNames[STATIC_MESH_MODEL_FLOOR]) {
			mRenderer->VBindMesh(modelCluster->mMesh);
			mRenderer->GetDeviceContext()->DrawIndexedInstanced(modelCluster->mMesh->GetIndexCount(), numElements, 0, 0, instanceCount);
		}
		instanceCount += numElements;

		for (auto i = 0; i < numElements; i++) ++it;
	}

	mRenderer->VCopySubresource(mGridContext, 4, 2);

	//Now only the floors
	instanceCount = 0;
	for (Factory<StaticMesh>::iterator it = Factory<StaticMesh>().begin(); it != Factory<StaticMesh>().end();)
	{
		auto& staticMesh = *it;
		auto modelCluster = staticMesh.mModel;
		auto numElements = modelCluster->ShareCount();

		if (modelCluster->mName == kStaticMeshModelNames[STATIC_MESH_MODEL_FLOOR]) {
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
	mRenderer->VSetPixelShader(mApplication->mPSFwdSpotLightVolume);

	mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, mCameraManager->GetOrigin().pCols, 1);
	mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, mCameraManager->GetCBufferFullLevelOrto(), 0);

	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
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
		mRenderer->VSetPixelShaderDepthResourceView(mShadowContext, i, 2);	// Shadow
		mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);		// Border

		mRenderer->VBindMesh(l.mConeMesh);

		mRenderer->VDrawIndexed(0, l.mConeMesh->GetIndexCount());
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

	//Copy previous results to a CPU friendly buffer
	mDeviceContext->CopyResource(mOutputDataCPURead, mOutputData);

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