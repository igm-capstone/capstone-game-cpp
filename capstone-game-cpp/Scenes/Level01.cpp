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
#include <Culler.h>
#include <SceneObjects/Lamp.h>

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
	mShadowContext(nullptr),
	mWallShaderResource(nullptr),
	mExplorerShaderResource(nullptr),
	mPLVShaderResource(nullptr)
{

}

Level01::~Level01()
{
	mWallMesh0->~IMesh();
	mPlaneMesh->~IMesh();
	mExplorerCubeMesh->~IMesh();
	mPLVMesh->~IMesh();
	mNDSQuadMesh->~IMesh();

	for (Explorer& e : Factory<Explorer>())
	{
		//e.mMesh->~IMesh();
	}
	mExplorerCubeMesh->~IMesh();

	mWallShaderResource->~IShaderResource();
	mExplorerShaderResource->~IShaderResource();
	mPLVShaderResource->~IShaderResource();
	
	mGBufferContext->~IRenderContext();
	mShadowContext->~IRenderContext();

	mAllocator.Free();
}

void Level01::VOnResize()
{
	// 0: Position, 1: Normal, 2: Color 3: Albedo
	mRenderer->VCreateContextResourceTargets(mGBufferContext, 4, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
	mRenderer->VCreateContextDepthStencilResourceTargets(mGBufferContext, 1, mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

	mRenderer->VCreateContextDepthStencilResourceTargets(mShadowContext, mPointLightCount, 2048, 2048);

	mCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 1000.0f));
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
	mCamera.SetViewMatrix(mat4f::lookAtLH(vec3f(0, 0, 0), vec3f(10.0f, 0.0f, -100.0f), vec3f(0, 1, 0))); //Temporary until Ghost get a controller.

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

	//mPointLightWorldMatrices[0] = (mat4f::scale(15.41f) * mat4f::translate(vec3f(0.0f, 0.0f, -1.0f))).transpose();
	//mPointLightCount = 3;
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

	Geometry::Plane(vertices, indices, mPlaneWidth, mPlaneHeight, 5, 5);

	meshLibrary.NewMesh(&mPlaneMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mPlaneMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mPlaneMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	// Point Light Volume 

//	Geometry::Cone(vertices, indices, 6, 1.0f, PI * 0.5f);

	OBJBasicResource<Vertex3> sphereResource("D:/Users/go4113/Capstone/Bin/Debug/spotlight_cone.obj");
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
	mRenderer->VCreateRenderContext(&mShadowContext, &mAllocator);

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
	size_t	cbWallSizes[] = { sizeof(CameraData) };

	mRenderer->VCreateShaderConstantBuffers(mWallShaderResource, cbWallData, cbWallSizes, 1);

	// Textures
	const char* filenames[] = { "D:/Users/go4113/Capstone/Bin/Debug/tileable5d.png", "D:/Users/go4113/Capstone/Bin/Debug/wood floor 2.png" };
	mRenderer->VAddShaderTextures2D(mWallShaderResource, filenames, 2);

	mRenderer->VAddShaderLinearSamplerState(mWallShaderResource, SAMPLER_STATE_ADDRESS_WRAP);

	// Explorers

	mRenderer->VCreateShaderResource(&mExplorerShaderResource, &mAllocator);

	void* cbExplorerData[] = { &mPVM };
	size_t cbExplorerSizes[] = { sizeof(CbufferPVM) };

	mRenderer->VCreateShaderConstantBuffers(mExplorerShaderResource, cbExplorerData, cbExplorerSizes, 1);

	// PLV

	mRenderer->VCreateShaderResource(&mPLVShaderResource, &mAllocator);

	void* cbPVLData[] = { &mLightData };
	size_t cbPVLSizes[] = { sizeof(CBufferLight) };

	mRenderer->VCreateShaderConstantBuffers(mPLVShaderResource, cbPVLData, cbPVLSizes, 1);

	mRenderer->VAddShaderLinearSamplerState(mPLVShaderResource, SAMPLER_STATE_ADDRESS_CLAMP);
	mRenderer->VAddShaderLinearSamplerState(mPLVShaderResource, SAMPLER_STATE_ADDRESS_BORDER, const_cast<float*>(Colors::transparent.pCols));

	mRenderer->AddAdditiveBlendState(mPLVShaderResource);
}
#pragma endregion

#pragma region Update

void Level01::VUpdate(double milliseconds)
{
	for (ExplorerController& ec : Factory<ExplorerController>())
	{
		ec.Update(milliseconds);
	}

	for (Skill& skill : Factory<Skill>())
	{
		skill.Update();
	}

	UpdateCamera();

	mCollisionManager.DetectCollisions();
	mCollisionManager.ResolveCollisions();

	mNetworkManager->Update();
}

void Level01::UpdateCamera()
{
	//uint32_t i = 0;
	//for (Lamp& l : Factory<Lamp>())
	//{
	//	mPVM.camera.projection = mat4f::normalizedPerspectiveLH(PI * 0.5f, mRenderer->GetAspectRatio(), 0.1f, l.mLightRadius).transpose();
	//	mPVM.camera.view = mat4f::lookToLH(vec3f(1.0f, 0.0f, 0.0f), mPointLightWorldMatrices[i].transpose().t, vec3f(0.0f, 0.0f, -1.0f)).transpose();
	//	if (i >= 1) break;
	//}

	mPVM.camera.projection	= mCamera.GetProjectionMatrix().transpose();
	mPVM.camera.view		= mCamera.GetViewMatrix().transpose();
}

#pragma endregion

#pragma region Render

void Level01::VRender()
{
	// Reset state.
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, Colors::transparent.pCols, 0xffffffff);

	RenderShadowMaps();

	RenderWalls();
	RenderExplorers();
	RenderPointLightVolumes();
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

void Level01::RenderShadowMaps()
{
	mRenderer->VSetInputLayout(mApplication->mExplorerVertexShader);
	mRenderer->VSetVertexShader(mApplication->mExplorerVertexShader);
	mRenderer->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 2048;
	viewport.Height = 2048;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	mRenderer->GetDeviceContext()->RSSetViewports(1, &viewport);

	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		if (i >= mPointLightCount)
		{
			break;
		}

		mRenderer->VSetRenderContextDepthTarget(mShadowContext, i);
		mRenderer->VClearDepthStencil(mShadowContext, i, 1.0f, 0);

		mat4f projection = mat4f::normalizedPerspectiveLH(PI * 0.5f, mRenderer->GetAspectRatio(), 0.1f, l.mLightRadius);
		mat4f view = mat4f::lookToLH(vec3f(1.0f, 0.0f, 0.0f), mPointLightWorldMatrices[i].transpose().t, vec3f(0.0f, 0.0f, -1.0f));

		std::vector<uint32_t> indices;
		Rig3D::Frustum frustum;
		mat4f viewProjection = view * projection;
		Rig3D::ExtractNormalizedFrustumLH(&frustum, viewProjection);

		mLightPVM.camera.projection = projection.transpose();
		mLightPVM.camera.view = view.transpose();

		// Walls

		CullWalls(frustum, indices);

		mRenderer->VBindMesh(mWallMesh0);

		for (uint32_t j : indices)
		{
			mLightPVM.world = mWallWorldMatrices0[j];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM, 0);
			mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
			mRenderer->VDrawIndexed(0, mWallMesh0->GetIndexCount());
		}

		indices.clear();

		// Planes

		CullPlanes(frustum, indices, mPlaneWorldMatrices, mPlaneWidth, mPlaneHeight, mPlaneCount);

		mRenderer->VBindMesh(mPlaneMesh);
		
		for (uint32_t j : indices)
		{
			mLightPVM.world = mPlaneWorldMatrices[j];
			mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mLightPVM, 0);
			mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
			mRenderer->VDrawIndexed(0, mPlaneMesh->GetIndexCount());
		}

		i++;
	}
}

void Level01::RenderWalls()
{
	mRenderer->GetDeviceContext()->RSSetViewports(1, &mRenderer->GetViewport());
	mRenderer->VSetRenderContextTargetsWithDepth(mGBufferContext, 0);

	mRenderer->VClearContextTarget(mGBufferContext, 0, Colors::magenta.pCols);	// Position
	mRenderer->VClearContextTarget(mGBufferContext, 1, Colors::magenta.pCols);	// Normal
	mRenderer->VClearContextTarget(mGBufferContext, 2, Colors::magenta.pCols);	// Color
	mRenderer->VClearDepthStencil(mGBufferContext, 0, 1.0f, 0);					// Depth

	mRenderer->VSetInputLayout(mApplication->mQuadVertexShader);
	mRenderer->VSetVertexShader(mApplication->mQuadVertexShader);
	mRenderer->VSetPixelShader(mApplication->mQuadPixelShader);

	// This can probably go into the render method...
	mRenderer->VUpdateShaderConstantBuffer(mWallShaderResource, &mPVM.camera, 0);

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
	//mRenderer->VSetContextTarget();
	//mRenderer->VClearContextTarget(Colors::black.pCols);

	mRenderer->VSetRenderContextTarget(mGBufferContext, 3);
	mRenderer->VClearContextTarget(mGBufferContext, 3, Colors::black.pCols);	// Albedo

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	mRenderer->SetBlendState(mPLVShaderResource, 0, color, 0xffffffff);

	mRenderer->VSetInputLayout(mApplication->mPLVolumeVertexShader);
	mRenderer->VSetVertexShader(mApplication->mPLVolumeVertexShader);
	mRenderer->VSetPixelShader(mApplication->mPLVolumePixelShader);

	mRenderer->VBindMesh(mPLVMesh);
	
	uint32_t i = 0;
	for (Lamp& l : Factory<Lamp>())
	{
		if (i >= mPointLightCount)
		{
			break;
		}

		mPVM.world = mPointLightWorldMatrices[i];

		mRenderer->VUpdateShaderConstantBuffer(mExplorerShaderResource, &mPVM, 0);

		mLightData.camera.projection = mat4f::normalizedPerspectiveLH(PI * 0.5f, mRenderer->GetAspectRatio(), 0.1f, l.mLightRadius).transpose();
		mLightData.camera.view = mat4f::lookToLH(vec3f(1.0f, 0.0f, 0.0f), mPointLightWorldMatrices[i].transpose().t, vec3f(0.0f, 0.0f, -1.0f)).transpose();
		mLightData.color = Colors::yellow;
		mLightData.range = l.mLightRadius;
		mLightData.cosAngle = cos(1.57079632679f);

		mRenderer->VUpdateShaderConstantBuffer(mPLVShaderResource, &mLightData, 0);

		mRenderer->VSetVertexShaderConstantBuffer(mExplorerShaderResource, 0, 0);
		mRenderer->VSetVertexShaderConstantBuffer(mPLVShaderResource, 0, 1);
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 0, 0);	// Position
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 1, 1);	// Normal
		mRenderer->VSetPixelShaderDepthResourceView(mShadowContext, i, 2);	// Shadow
		mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);	// Clamp, Border
		
		mRenderer->VDrawIndexed(0, mPLVMesh->GetIndexCount());
		i++;
	}

	

	mRenderer->GetDeviceContext()->OMSetBlendState(nullptr, color, 0xffffffff);
}

void Level01::RenderFullScreenQuad()
{
	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(Colors::magenta.pCols, 1.0f, 0);

	static int state = 1;
	if (mEngine->GetInput()->GetKeyDown(KEYCODE_G))
	{
		state = 0;
	}
	else if (mEngine->GetInput()->GetKeyDown(KEYCODE_N))
	{
		state = 1;
	}
	else if (mEngine->GetInput()->GetKeyDown(KEYCODE_D))
	{
		state = 2;
	}

	if (state == 0)
	{
		mRenderer->VSetInputLayout(mApplication->mNDSQuadVertexShader);
		mRenderer->VSetVertexShader(mApplication->mNDSQuadVertexShader);
		mRenderer->VSetPixelShader(mApplication->mDBGPixelShader);

		static uint32_t index = 0;
		if (mEngine->GetInput()->GetKeyDown(KEYCODE_0))
		{
			index = 0;
		}
		else if (mEngine->GetInput()->GetKeyDown(KEYCODE_1))
		{
			index = 1;
		}
		else if (mEngine->GetInput()->GetKeyDown(KEYCODE_2))
		{
			index = 2;
		}
		else if (mEngine->GetInput()->GetKeyDown(KEYCODE_3))
		{
			index = 3;
		}
		else if (mEngine->GetInput()->GetKeyDown(KEYCODE_4))
		{
			index = 4;
		}

		if (index < 4)
		{
			mRenderer->VSetPixelShaderResourceView(mGBufferContext, index, 0);		// Color
			mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);
		}
		else
		{
			mRenderer->VSetPixelShaderDepthResourceView(mGBufferContext, 0, 0);		// Depth
			mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);
		}
		
	}
	else if (state == 2)
	{
		static uint32_t d = 0;
		if (mEngine->GetInput()->GetKeyDown(KEYCODE_RIGHT))
		{
			d = min(mPointLightCount - 1, d + 1);
		}
		else if (mEngine->GetInput()->GetKeyDown(KEYCODE_RIGHT))
		{
			d = max(0, d - 1);
		}

		mRenderer->VSetPixelShaderDepthResourceView(mShadowContext, d, 0);		// Color
		mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);
	}
	else
	{
		mRenderer->VSetInputLayout(mApplication->mNDSQuadVertexShader);
		mRenderer->VSetVertexShader(mApplication->mNDSQuadVertexShader);
		mRenderer->VSetPixelShader(mApplication->mNDSQuadPixelShader);

		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 2, 0);		// Color
		mRenderer->VSetPixelShaderResourceView(mGBufferContext, 3, 1);		// Albedo
		mRenderer->VSetPixelShaderDepthResourceView(mGBufferContext, 0, 2);	// Depth
		mRenderer->VSetPixelShaderSamplerStates(mPLVShaderResource);
	}

	mRenderer->VBindMesh(mNDSQuadMesh);
	mRenderer->VDrawIndexed(0, mNDSQuadMesh->GetIndexCount());
}

#pragma endregion

void Level01::VShutdown()
{
	
}