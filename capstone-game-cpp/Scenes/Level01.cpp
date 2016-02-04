#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Level01.h"
#include "MainMenuScene.h"
#include <Resource.h>
#include <Colors.h>
#include <Vertex.h>
#include <Rig3D/Geometry.h>
#include <Rig3D/Graphics/Interface/IShaderResource.h>
#include <SceneObjects/Explorer.h>
#include <Rig3D/Graphics/DirectX11/DX11IMGUI.h>
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>
#include <Rig3D/Graphics/Interface/IRenderContext.h>

#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>
#include <Rig3D/Graphics/DirectX11/DX11IMGUI.h>
#include <Console.h>

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

Level01::Level01() :
	mWallCount0(0),
	mPlaneCount(0),
	mPointLightCount(0),
	mExplorerCount(0),
	mPlaneWidth(0.0f),
	mPlaneHeight(0.0f),
	mWallWorldMatrices0(nullptr),
	mPlaneWorldMatrices(nullptr),
	mPointLightWorldMatrices(nullptr),
	mPointLightColors(nullptr),
	mWallMesh0(nullptr),
	mPlaneMesh(nullptr),
	mExplorerCubeMesh(nullptr),
	mPLVMesh(nullptr),
	mNDSQuadMesh(nullptr),
	mGBufferContext(nullptr),
	mWallShaderResource(nullptr),
	mExplorerShaderResource(nullptr),
	mPLVShaderResource(nullptr)
{
	auto e = Factory<Explorer>::Create();
	e->mController->mIsActive = true;
}

Level01::~Level01()
{
	mWallMesh0->~IMesh();
	mPlaneMesh->~IMesh();
	mExplorerCubeMesh->~IMesh();
	mPLVMesh->~IMesh();
	mNDSQuadMesh->~IMesh();

	mWallShaderResource->~IShaderResource();
	mExplorerShaderResource->~IShaderResource();
	mPLVShaderResource->~IShaderResource();
	
	mGBufferContext->~IRenderContext();

	mAllocator.Free();
}

void Level01::VOnResize()
{
	mMainCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 1000.0f));

	// 0: Position, 1: Normal, 2: Color 3: Albedo
	mRenderer->VCreateContextResourceTargets(mGBufferContext, 4, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
	mRenderer->VCreateContextDepthStencilResourceTargets(mGBufferContext, 1, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
}

#pragma region Initialization

void Level01::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	mAllocator.SetMemory(mStaticMemory, mStaticMemory + mStaticMemorySize);
	mRenderer->SetDelegate(this);

	InitializeResource();
	InitializeGeometry();
	InitializeShaderResources();
	InitializeMainCamera();

	mCollisionManager.Initialize();

	VOnResize();

	mState = BASE_SCENE_STATE_RUNNING;
}

void Level01::InitializeResource()
{
	auto level = Resource::LoadLevel("Assets/Level01.json", mAllocator);

	mWallCount0 = level.wallCount;
	mWallWorldMatrices0 = level.walls;

	mPlaneCount = level.tileCount;
	mPlaneWorldMatrices = level.tiles;
	mPlaneWidth = level.tileWidth;
	mPlaneHeight = level.tileHeight;

	mPointLightCount = level.lampCount;
	mPointLightWorldMatrices = level.lamps;
	mPointLightColors = level.lampColors;
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

	meshLibrary.NewMesh(&mExplorerCubeMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mExplorerCubeMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mExplorerCubeMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	Geometry::Plane(vertices, indices, mPlaneWidth, mPlaneHeight, 5.0f, 5.0f);

	meshLibrary.NewMesh(&mPlaneMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mPlaneMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mPlaneMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Point Light Volume 

	Geometry::Sphere(vertices, indices, 6, 6, 1.0f);

	OBJBasicResource<Vertex3> sphereResource("D:/Users/go4113/Capstone/Bin/Debug/sphere.obj");
	sphereResource.Load();
	meshLibrary.LoadMesh(&mPLVMesh, mRenderer, sphereResource);

	//meshLibrary.NewMesh(&mPLVMesh, mRenderer);
	//mRenderer->VSetMeshVertexBuffer(mPLVMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	//mRenderer->VSetMeshIndexBuffer(mPLVMesh, &indices[0], indices.size());

	// Full Screen Quad

	indices.clear();

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
	void*	cbWallData[] = { &mPVM.camera };
	size_t	cbWallSizes[] = { sizeof(CbufferPVM::CameraData) };

	mRenderer->VCreateShaderConstantBuffers(mWallShaderResource, cbWallData, cbWallSizes, 1);

	// Explorers

	mRenderer->VCreateShaderResource(&mExplorerShaderResource, &mAllocator);

	void* cbExplorerData[] = { &mPVM };
	size_t cbExplorerSizes[] = { sizeof(CbufferPVM) };

	mRenderer->VCreateShaderConstantBuffers(mExplorerShaderResource, cbExplorerData, cbExplorerSizes, 1);

	// PLV

	mRenderer->VCreateShaderResource(&mPLVShaderResource, &mAllocator);

	// Instance buffer data
	void*	ibLampData[] = { mPointLightColors, mPointLightWorldMatrices};
	size_t	ibLampSizes[] = {  sizeof(vec4f) * mPointLightCount, sizeof(mat4f) * mPointLightCount };
	size_t	ibLampStrides[] = { sizeof(vec4f), sizeof(mat4f) };
	size_t	ibLampOffsets[] = { 0, 0 };

	mRenderer->VCreateDynamicShaderInstanceBuffers(mPLVShaderResource, ibLampData, ibLampSizes, ibLampStrides, ibLampOffsets, 2);

	mRenderer->VUpdateShaderInstanceBuffer(mPLVShaderResource, mPointLightColors, ibLampSizes[0], 0);
	mRenderer->VUpdateShaderInstanceBuffer(mPLVShaderResource, mPointLightWorldMatrices, ibLampSizes[1], 1);

	mRenderer->VAddShaderLinearSamplerState(mPLVShaderResource, SAMPLER_STATE_ADDRESS_CLAMP);
	mRenderer->AddAdditiveBlendState(mPLVShaderResource);
}

void Level01::InitializeMainCamera()
{
	// Use to set view based on network logic
	mMainCamera.SetViewMatrix(mat4f::lookAtLH(kVectorZero, vec3f(10.0f, 0.0f, -100.0f), kVectorUp));
}
#pragma endregion

#pragma region Update

void Level01::VUpdate(double milliseconds)
{
	for (ExplorerController& ec : Factory<ExplorerController>())
	{
		ec.Update();
	}

	UpdateCamera();

	mCollisionManager.DetectCollisions();
	mCollisionManager.ResolveCollisions();

	mNetworkManager->Update();
}

void Level01::UpdateCamera()
{
	mPVM.camera.projection	= mMainCamera.GetProjectionMatrix().transpose();
	mPVM.camera.view		= mMainCamera.GetViewMatrix().transpose();
}

#pragma endregion

#pragma region Render

void Level01::VRender()
{
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);

	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

	SetGBufferRenderTargets();

	RenderWalls();
	RenderExplorers();
	RenderPointLightVolumes();

	SetDefaultTarget();

	RenderFullScreenQuad();

	ID3D11ShaderResourceView* nullSRV[4] = { 0, 0, 0, 0 };
	mRenderer->GetDeviceContext()->PSSetShaderResources(0, 4, nullSRV);

	//FPS
	mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr);

	DX11IMGUI::NewFrame();
	RenderFPSIndicator();
	Console::Draw();
	ImGui::Render();

	mRenderer->VSwapBuffers();
}

void Level01::SetGBufferRenderTargets()
{
	mRenderer->VSetRenderContextTargetsWithDepth(mGBufferContext, 0);

	mRenderer->VClearContextTarget(mGBufferContext, 0, Colors::magenta.pCols);	// Position
	mRenderer->VClearContextTarget(mGBufferContext, 1, Colors::magenta.pCols);	// Normal
	mRenderer->VClearContextTarget(mGBufferContext, 2, Colors::magenta.pCols);	// Color
	mRenderer->VClearDepthStencil(mGBufferContext, 0, 1.0f, 0);					// Depth
}

void Level01::SetDefaultTarget()
{
	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(Colors::magenta.pCols, 1.0f, 0);
}

void Level01::RenderWalls()
{
	mRenderer->VSetInputLayout(mApplication->mQuadVertexShader);
	mRenderer->VSetVertexShader(mApplication->mQuadVertexShader);
	mRenderer->VSetPixelShader(mApplication->mQuadPixelShader);

	// This can probably go into the render method...
	mRenderer->VUpdateShaderConstantBuffer(mWallShaderResource, &mPVM.camera, 0);

	mRenderer->VBindMesh(mWallMesh0);
	mRenderer->VSetVertexShaderInstanceBuffer(mWallShaderResource, 0, 1);
	mRenderer->VSetVertexShaderConstantBuffer(mWallShaderResource, 0, 0);

	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mWallMesh0->GetIndexCount(), mWallCount0, 0, 0, 0);

	mRenderer->VBindMesh(mPlaneMesh);
	mRenderer->VSetVertexShaderInstanceBuffer(mWallShaderResource, 1, 1);

	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mPlaneMesh->GetIndexCount(), mPlaneCount, 0, 0, 0);
}

void Level01::RenderExplorers()
{
	mRenderer->VSetInputLayout(mApplication->mExplorerVertexShader);
	mRenderer->VSetVertexShader(mApplication->mExplorerVertexShader);
	mRenderer->VSetPixelShader(mApplication->mExplorerPixelShader);

	for (Explorer& e : Factory<Explorer>())
	{
		mPVM.world = e.mTransform->GetWorldMatrix().transpose();
		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mPVM, 0);

		mRenderer->VBindMesh(mExplorerCubeMesh);
		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
		mRenderer->VDrawIndexed(0, mExplorerCubeMesh->GetIndexCount());
	}
}

void Level01::RenderPointLightVolumes()
{
	mRenderer->VSetRenderContextTarget(mGBufferContext, 3);
	mRenderer->VClearContextTarget(mGBufferContext, 3, Colors::magenta.pCols);	// Albedo

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);

	mRenderer->VSetInputLayout(mApplication->mPLVolumeVertexShader);
	mRenderer->VSetVertexShader(mApplication->mPLVolumeVertexShader);
	mRenderer->VSetPixelShader(mApplication->mPLVolumePixelShader);

	mRenderer->VBindMesh(mPLVMesh);
	mRenderer->VSetVertexShaderInstanceBuffer(mPLVShaderResource, 0, 1);	// Colors
	mRenderer->VSetVertexShaderInstanceBuffer(mPLVShaderResource, 1, 2);	// World Matrices
	mRenderer->VSetVertexShaderConstantBuffer(mWallShaderResource, 0, 0);
	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 0, 0);	// Position
	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 1, 1);	// Normal
	mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);

	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mPLVMesh->GetIndexCount(), mPointLightCount, 0, 0, 0);

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, color, 0xffffffff);
}

void Level01::RenderFullScreenQuad()
{
	//mRenderer->VSetInputLayout(mApplication->mNDSQuadVertexShader);
	//mRenderer->VSetVertexShader(mApplication->mNDSQuadVertexShader);
	//mRenderer->VSetPixelShader(mApplication->mDBGPixelShader);

	//static uint32_t index = 0;
	//if (mEngine->GetInput()->GetKeyDown(KEYCODE_0))
	//{
	//	index = 0;
	//}
	//else if (mEngine->GetInput()->GetKeyDown(KEYCODE_1))
	//{
	//	index = 1;
	//}
	//else if (mEngine->GetInput()->GetKeyDown(KEYCODE_2))
	//{
	//	index = 2;
	//}
	//else if (mEngine->GetInput()->GetKeyDown(KEYCODE_3))
	//{
	//	index = 3;
	//}

	//mRenderer->VSetPixelShaderResourceView(mGBufferContext, index, 0);		// Color
	//mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);


	mRenderer->VSetInputLayout(mApplication->mNDSQuadVertexShader);
	mRenderer->VSetVertexShader(mApplication->mNDSQuadVertexShader);
	mRenderer->VSetPixelShader(mApplication->mNDSQuadPixelShader);

	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 2, 0);		// Color
	mRenderer->VSetPixelShaderResourceView(mGBufferContext, 3, 1);		// Albedo
	mRenderer->VSetPixelShaderDepthResourceView(mGBufferContext, 0, 3);	// Depth
	mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);

	mRenderer->VBindMesh(mNDSQuadMesh);
	mRenderer->VDrawIndexed(0, mNDSQuadMesh->GetIndexCount());
}

#pragma endregion

void Level01::VShutdown()
{
	
}