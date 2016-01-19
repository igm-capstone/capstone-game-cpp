#pragma once
#include "Graphics/Interface/IScene.h"
#include "Memory/Memory/Memory.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

#define STATIC_SCENE_MEMORY 1000000

namespace Rig3D
{
	class RIG3D Application
	{
	private:
		LinearAllocator mSceneAllocator;	// Not sure if this is the best guy for the job, but we will see.
		char*			mStaticMemory;
		size_t			mStaticMemorySize;

	protected:
		Application() : 
			mSceneAllocator(),	
			mStaticMemory(nullptr),
			mStaticMemorySize(0),
			mLoadingScene(nullptr),
			mCurrentScene(nullptr),
			mSceneToLoad(nullptr),
			unload(false)
		{
			
		}
		
		~Application()
		{
			
		}

		Application(Application const&) = delete;
		void operator=(Application const&) = delete;
	public:
		Options mOptions;
		IScene* mLoadingScene;
		IScene* mCurrentScene;
		IScene* mSceneToLoad;
		bool unload;

		void SetStaticMemory(void* start, size_t size)
		{
			mStaticMemory = static_cast<char*>(start);
			mStaticMemorySize = size;
			mSceneAllocator.SetMemory(start, mStaticMemory + STATIC_SCENE_MEMORY);
		}

		void UpdateCurrentScene()
		{
			// TODO: Review logic
			if (mSceneToLoad == nullptr)
			{
				if (unload)
				{
					if (mCurrentScene != nullptr)
					{
						mCurrentScene->VShutdown();
						mSceneAllocator.Free();
					}
				}

				return;
			}

			if (mCurrentScene != nullptr)
			{
				mCurrentScene->VShutdown();
				mSceneAllocator.Free();
			}

			// free current scene????
			mCurrentScene = mSceneToLoad;
			mSceneToLoad = nullptr;

			mCurrentScene->VInitialize();
		}

		template <class TScene>
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
			asScene = new (mSceneAllocator.Allocate(sizeof(TScene), alignof(TScene), 0)) TScene();

			// Pass the remainer to the scene constructor
			size_t size = (mStaticMemory + STATIC_SCENE_MEMORY) - (asChar + sizeof(TScene));
			asScene->SetStaticMemory(mSceneAllocator.Allocate(size, 2, 0), size);

			mSceneToLoad = asScene;
		}

		void UnloadScene()
		{
			unload = true;
		}

		void Update(float deltaTime)
		{
			IScene* scene = mCurrentScene;
			if (scene == nullptr)
			{
				scene = mLoadingScene;
			}

			scene->VUpdate(deltaTime);
			scene->VRender();
		}
	};
}