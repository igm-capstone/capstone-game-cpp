#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"

using namespace Rig3D;

ScareTacticsApplication::ScareTacticsApplication() :
	mLoadingScreen(nullptr),
	mCurrentScene(nullptr),
	mSceneToLoad(nullptr),
	mSceneAllocator(),
	mStaticMemory(nullptr),
	mStaticMemorySize(0)
{
	
}

ScareTacticsApplication::~ScareTacticsApplication()
{

}

void ScareTacticsApplication::SetLoadingScreen(BaseScene* loading)
{
	mLoadingScreen = loading;
}

void ScareTacticsApplication::SetStaticMemory(void* start, size_t size)
{
	mStaticMemory = static_cast<char*>(start);
	mStaticMemorySize = size;
	mSceneAllocator.SetMemory(start, mStaticMemory + STATIC_SCENE_MEMORY + 6);
}

IMesh* ScareTacticsApplication::GetPrimitive(Primitive primitive)
{
	switch (primitive)
	{
	case Primitive::Quad:
		return mQuadMesh;
	case Primitive::Sphere:
		return mSphereMesh;
	case Primitive::Cube:
		return mCubeMesh;
	default:
		return nullptr;
	}
}

void ScareTacticsApplication::UnloadScene()
{
	if (mCurrentScene)
	{
		mCurrentScene->VShutdown();
		mCurrentScene->~BaseScene();
		mSceneAllocator.Reset();
		_aligned_free(mCurrentScene);
		mCurrentScene = nullptr;
	}
}

void ScareTacticsApplication::CreatePrimitives()
{
	// TODO create mesh library
	MeshLibrary<LinearAllocator> meshLibrary;

	// Quad
	vec3f quadVertices[4] = {
		{ +1.0f, -1.0f, 0.0f },
		{ +1.0f, +1.0f, 0.0f },
		{ -1.0f, -1.0f, 0.0f },
		{ -1.0f, +1.0f, 0.0f }
	};

	uint16_t quadIndices[6] = { 
		0, 2, 1, 
		3, 2, 0
	};

	auto engine = &Singleton<Engine>::SharedInstance();
	auto renderer = engine->GetRenderer();

	meshLibrary.NewMesh(&mQuadMesh, renderer);
	renderer->VSetStaticMeshVertexBuffer(mQuadMesh, quadVertices, sizeof(vec3f) * 4, sizeof(vec3f));
	renderer->VSetStaticMeshIndexBuffer(mQuadMesh, quadIndices, 6);
}

void ScareTacticsApplication::VInitialize()
{
	//CreatePrimitives();
	mLoadingScreen->VInitialize();
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
		
		mSceneToLoad->mApplication = this;
		mSceneToLoad->SetStaticMemory(mSceneAllocator.Allocate(STATIC_SCENE_MEMORY, 2, 0), STATIC_SCENE_MEMORY);
		mSceneToLoad->VInitialize();	// Once asychronous this function should return immediately
	}
	else if (mSceneToLoad->mState == BASE_SCENE_STATE_RUNNING)
	{
		mCurrentScene = mSceneToLoad;
		mSceneToLoad = nullptr;
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
		mLoadingScreen->VUpdate(deltaTime);
		mLoadingScreen->VRender();
	}
}

void ScareTacticsApplication::VShutdown()
{
	if (mCurrentScene)
	{
		mCurrentScene->~BaseScene();
	}

	if (mLoadingScreen)
	{
		mLoadingScreen->~BaseScene();
	}

	if (mSceneToLoad)
	{
		mSceneToLoad->~BaseScene();
	}

	mSceneAllocator.Free();
}