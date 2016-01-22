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

	InitializeShaderResources();

	mState = BASE_SCENE_STATE_RUNNING;
}


void Level01::InitializeShaderResources()
{

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