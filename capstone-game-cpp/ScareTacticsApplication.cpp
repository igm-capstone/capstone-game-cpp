#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"

using namespace Rig3D;

ScareTacticsApplication::ScareTacticsApplication() :
	mSceneAllocator(),
	mStaticMemory(nullptr),
	mStaticMemorySize(0),
	mLoadingScene(nullptr),
	mCurrentScene(nullptr),
	mSceneToLoad(nullptr),
	unload(false)
{
	
}

ScareTacticsApplication::~ScareTacticsApplication()
{

}

void ScareTacticsApplication::SetStaticMemory(void* start, size_t size)
{
	mStaticMemory = static_cast<char*>(start);
	mStaticMemorySize = size;
	mSceneAllocator.SetMemory(start, mStaticMemory + STATIC_SCENE_MEMORY);
}

void ScareTacticsApplication::UnloadScene()
{
	unload = true;

	if (mCurrentScene)
	{
		mCurrentScene->VShutdown();
		mCurrentScene->~BaseScene();
		_aligned_free(mCurrentScene);
		mCurrentScene = nullptr;
	}
}

void ScareTacticsApplication::VInitialize()
{
	mLoadingScene->VInitialize();
}

void ScareTacticsApplication::VUpdateCurrentScene()
{
	// No scene to load: Keep running current scene
	if (mSceneToLoad == nullptr)
	{
		return;
	}

	if (mSceneToLoad->mState == BASE_SCENE_STATE_CONSTRUCTED)
	{
		UnloadScene();
		
		mSceneToLoad->VInitialize();	// Once asychronous this function should return immediately
	}
	else if (mSceneToLoad->mState == BASE_SCENE_STATE_RUNNING)
	{
		mCurrentScene = mSceneToLoad;
		mSceneToLoad == nullptr;
	}
}

void ScareTacticsApplication::VUpdate(float deltaTime)
{
	if (mCurrentScene)
	{
		mCurrentScene->VUpdate(deltaTime);
		mCurrentScene->VRender();
	}
	else
	{
		mLoadingScene->VUpdate(deltaTime);
		mLoadingScene->VRender();
	}
		//BaseScene* scene = mCurrentScene;
		//if (scene == nullptr)
		//{
		//	scene = mLoadingScene;
		//}

		//scene->VUpdate(deltaTime);
		//scene->VRender();
}

void ScareTacticsApplication::VShutdown()
{
	if (mCurrentScene)
	{
		mCurrentScene->~BaseScene();
	}

	if (mLoadingScene)
	{
		mLoadingScene->~BaseScene();
	}

	if (mSceneToLoad)
	{
		mSceneToLoad->~BaseScene();
	}

	mSceneAllocator.Free();
}