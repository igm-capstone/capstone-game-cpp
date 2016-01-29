#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Level01.h"
#include "MainMenuScene.h"
#include <Resource.h>
#include <Colors.h>

void Level01::VOnResize()
{
}

#pragma region Initialization

void Level01::VInitialize()
{
	LinearAllocator allocator(mStaticMemory, mStaticMemory + mStaticMemorySize);

	mState = BASE_SCENE_STATE_INITIALIZING;

	auto level = Resource::LoadLevel("Assets/Level01.json", allocator);
	mWallCount = level.wallCount;
	mWalls = level.walls;

	InitializeShaderResources();

	mCollisionManager.Initialize();

	mState = BASE_SCENE_STATE_RUNNING;
}


void Level01::InitializeShaderResources()
{
	D3D11_BUFFER_DESC quadInstanceBufferDesc;
	quadInstanceBufferDesc.ByteWidth = sizeof(mat4f) * mWallCount;
	quadInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	quadInstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	quadInstanceBufferDesc.CPUAccessFlags = 0;
	quadInstanceBufferDesc.MiscFlags = 0;
	quadInstanceBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	instanceData.pSysMem = mWalls;

	mDevice->CreateBuffer(&quadInstanceBufferDesc, &instanceData, &mWallInstanceBuffer);
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

#pragma endregion

void Level01::VShutdown()
{
}