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
}

void ScareTacticsApplication::VInitialize()
{
	mLoadingScene->VInitialize();
}

void ScareTacticsApplication::VUpdateCurrentScene()
{
	if (mCurrentScene == nullptr)
	{
		return;
	}

	//if (mCurrentScene->mState == BASE_SCENE_STATE_RUNNING)
	//{
	//	mCurrentScene = mSceneToLoad;
	//	mSceneToLoad = nullptr;
	//}
}

void ScareTacticsApplication::VUpdate(float deltaTime)
{
	if (mCurrentScene->mState == BASE_SCENE_STATE_RUNNING)
	{
		mCurrentScene->VUpdate(deltaTime);
		mCurrentScene->VRender();
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
	mCurrentScene->~BaseScene();
	mLoadingScene->~BaseScene();
	mSceneToLoad->~BaseScene();

	mSceneAllocator.Free();
}