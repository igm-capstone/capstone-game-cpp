#pragma once
#include "Rig3D/IApplication.h"
#include "capstone-game-cpp/Scenes/BaseScene.h"
#include <Rig3D/Singleton.h>
#include <assert.h>

using namespace Rig3D;

class ScareTacticsApplication : public IApplication
{
public:
	BaseScene* mLoadingScene;
	BaseScene* mCurrentScene;
	BaseScene* mSceneToLoad;

	ScareTacticsApplication();
	~ScareTacticsApplication();

	void SetStaticMemory(void* start, size_t size);

	void VInitialize() override;
	void VUpdateCurrentScene() override;
	void VUpdate(float deltaTime) override;
	void VShutdown() override;

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

private:
	LinearAllocator mSceneAllocator;	// Not sure if this is the best guy for the job, but we will see.
	char*			mStaticMemory;
	size_t			mStaticMemorySize;
};

typedef Singleton<ScareTacticsApplication> Application;