#pragma once
#include "Rig3D/IApplication.h"
#include "capstone-game-cpp/Scenes/BaseScene.h"
#include <Rig3D/Singleton.h>
#include <assert.h>
#include <unordered_map>
#include <Rig3D/Graphics/Interface/IShader.h>

using namespace Rig3D;

class ScareTacticsApplication : public IApplication
{
public:
	std::unordered_map<const char*, IShader*> mShaderMap;

	ScareTacticsApplication();
	~ScareTacticsApplication();

	void SetLoadingScreen(BaseScene* loading);
	void SetStaticMemory(void* start, size_t size);

	void VInitialize() override;
	void VUpdateCurrentScene() override;
	void VUpdate(float deltaTime) override;
	void VShutdown() override;

	void InitializeShaders();

	template<class TScene>
	void LoadScene()
	{
		// Verify our memory limits
		assert(sizeof(TScene) < STATIC_SCENE_MEMORY);

		union
		{
			TScene* asScene;
			char*	asChar;
		};

		// Allocate scene 
		asScene = new (_aligned_malloc(sizeof(TScene), alignof(TScene))) TScene();
		
		//asScene = new (mSceneAllocator.Allocate(sizeof(TScene), alignof(TScene), 0)) TScene();

		// Pass the remainer to the scene constructor
	//	size_t size = (mStaticMemory + STATIC_SCENE_MEMORY) - (asChar + sizeof(TScene));
	//	asScene->SetStaticMemory(mSceneAllocator.Allocate(size, 2, 0), size);

		mSceneToLoad = asScene;
	}

	void UnloadScene();
	BaseScene*	GetCurrentScene() const { return mCurrentScene; }

private:
	BaseScene*		mLoadingScreen;
	BaseScene*		mCurrentScene;
	BaseScene*		mSceneToLoad;

	LinearAllocator mGameAllocator;
	LinearAllocator mSceneAllocator;	// Not sure if this is the best guy for the job, but we will see.
	PoolAllocator	mShaderAllocator;
	PoolAllocator	mMeshAllocator;

	char*			mStaticMemory;
	size_t			mStaticMemorySize;
};

typedef Singleton<ScareTacticsApplication> Application;