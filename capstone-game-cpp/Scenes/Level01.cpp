#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Level01.h"
#include "MainMenuScene.h"
#include <Resource.h>
#include <Colors.h>
#include <Vertex.h>
#include <Rig3D/Geometry.h>
#include <Rig3D/Graphics/Interface/IShaderResource.h>

#define PI 3.14159265359f

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

Level01::Level01() : mWallCount0(0), mWallWorldMatrices0(nullptr), mWallMesh0(nullptr), mWallShaderResource(nullptr)
{
	
}

Level01::~Level01()
{
	
}

void Level01::VOnResize()
{
	mMainCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 1000.0f));
}

#pragma region Initialization

void Level01::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	mAllocator.SetMemory(mStaticMemory, mStaticMemory + mStaticMemorySize);
	mRenderer->SetDelegate(this);

	auto level = Resource::LoadLevel("Assets/Level01.json", mAllocator);
	mWallCount0 = level.wallCount;
	mWallWorldMatrices0 = level.walls;

	InitializeGeometry();
	InitializeShaderResources();
	InitializeMainCamera();

	mCollisionManager.Initialize();

	VOnResize();

	mState = BASE_SCENE_STATE_RUNNING;
}

void Level01::InitializeGeometry()
{
	MeshLibrary<LinearAllocator> meshLibrary(&mAllocator);
	meshLibrary.NewMesh(&mWallMesh0, mRenderer);

	std::vector<Vertex3> vertices;
	std::vector<uint16_t> indices;

	Geometry::Cube(vertices, indices, 2);

	

	mRenderer->VSetMeshVertexBuffer(mWallMesh0, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mWallMesh0, &indices[0], indices.size());
}

void Level01::InitializeShaderResources()
{
	// Allocate shader resource
	mRenderer->VCreateShaderResource(&mWallShaderResource, &mAllocator);

	// Instance buffer data
	void*	ibData[]		= { &mWallWorldMatrices0 };
	size_t	ibSizes[]		= { sizeof(mat4f) * mWallCount0 };
	size_t	ibStrides[]		= { sizeof(mat4f) };
	size_t	ibOffsets[]		= { 0 };

	// Create the instance buffer
	mRenderer->VCreateDynamicShaderInstanceBuffers(mWallShaderResource, ibData, ibSizes, ibStrides, ibOffsets, 1);
	
	// Set data for instance buffer once
	mRenderer->VUpdateShaderInstanceBuffer(mWallShaderResource, mWallWorldMatrices0, ibSizes[0], 0);

	// Constant buffer data
	void*	cbData[] = { &mPVM.camera };
	size_t	cbSizes[] = { sizeof(CbufferPVM::CameraData) };

	mRenderer->VCreateShaderConstantBuffers(mWallShaderResource, cbData, cbSizes, 1);
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
	UpdateCamera();
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
	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(Colors::magenta.pCols, 1.0f, 0);

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);

	RenderWalls();

	mRenderer->VSwapBuffers();
}

void Level01::RenderWalls()
{
	mRenderer->VSetInputLayout(mApplication->mQuadVertexShader);
	mRenderer->VSetVertexShader(mApplication->mQuadVertexShader);
	mRenderer->VSetPixelShader(mApplication->mQuadPixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mWallShaderResource, &mPVM.camera, 0);

	mRenderer->VBindMesh(mWallMesh0);
	mRenderer->VSetVertexShaderInstanceBuffer(mWallShaderResource, 0, 1);
	mRenderer->VSetVertexShaderConstantBuffer(mWallShaderResource, 0, 0);

	mRenderer->GetDeviceContext()->DrawIndexedInstanced(mWallMesh0->GetIndexCount(), mWallCount0, 0, 0, 0);
}

void Level01::RenderExplorers()
{
	
}


#pragma endregion

void Level01::VShutdown()
{
	mWallMesh0->~IMesh();
	mWallShaderResource->~IShaderResource();

	mAllocator.Free();
}