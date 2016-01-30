#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Level01.h"
#include "MainMenuScene.h"
#include <Resource.h>
#include <Colors.h>

#define PI 3.14159265359f

static const vec3f kVectorZero	= { 0.0f, 0.0f, 0.0f };
static const vec3f kVectorUp	= { 0.0f, 1.0f, 0.0f };

void Level01::VOnResize()
{
	mMainCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 100.0f));
}

#pragma region Initialization

void Level01::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	mAllocator.SetMemory(mStaticMemory, mStaticMemory + mStaticMemorySize);

	auto level = Resource::LoadLevel("Assets/Level01.json", mAllocator);
	mWallCount0 = level.wallCount;
	mWallWorldMatrices0 = level.walls;

	InitializeGeometry();
	InitializeShaderResources();
	InitializeMainCamera();

	mCollisionManager.Initialize();

	mState = BASE_SCENE_STATE_RUNNING;
}

void Level01::InitializeGeometry()
{
	MeshLibrary<LinearAllocator> meshLibrary(&mAllocator);
	meshLibrary.NewMesh(&mWallMesh0, mRenderer);


}

void Level01::InitializeShaderResources()
{
	// Allocate shader resource
	mRenderer->VCreateShaderResource(&mWallShaderResource, &mAllocator);

	void*	ibData[]		= { &mWallWorldMatrices0 };
	size_t	ibSizes[]		= { sizeof(mat4f) * mWallCount0 };
	size_t	ibStrides[]		= { sizeof(mat4f) };
	size_t	ibOffsets[]		= { 0 };

	// Create the instance buffer
	mRenderer->VCreateDynamicShaderInstanceBuffers(mWallShaderResource, ibData, ibSizes, ibStrides, ibOffsets, 1);
	
	// Set data for instance buffer once
	mRenderer->VUpdateShaderInstanceBuffer(mWallShaderResource, mWallWorldMatrices0, ibSizes[0], 0);
}

void Level01::InitializeMainCamera()
{
	// Use to set view based on network logic
	mMainCamera.SetViewMatrix(mat4f::lookAtLH(kVectorZero, vec3f(0.0f, 0.0f, -20.0f), kVectorUp));
}
#pragma endregion

#pragma region Update

void Level01::VUpdate(double milliseconds)
{
}

#pragma endregion

#pragma region Render

void Level01::VRender()
{
	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(Colors::magenta.pCols, 1.0f, 0);

	mRenderer->VSwapBuffers();
}

void Level01::RenderWalls()
{
	
}

#pragma endregion

void Level01::VShutdown()
{
}