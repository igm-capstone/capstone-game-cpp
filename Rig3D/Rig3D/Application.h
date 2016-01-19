#pragma once
#include "Graphics/Interface/IScene.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D Application
	{
	protected:
		Application();
		~Application();
		Application(Application const&) = delete;
		void operator=(Application const&) = delete;
	public:
		Options mOptions;
		IScene* mLoadingScene;
		IScene* mCurrentScene;
		IScene* mSceneToLoad;

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
					}

					// free current scene????
					mCurrentScene = mSceneToLoad;
				}

				return;
			}

			if (mCurrentScene != nullptr)
			{
				mCurrentScene->VShutdown();
			}

			// free current scene????
			mCurrentScene = mSceneToLoad;
			mSceneToLoad = nullptr;

			mCurrentScene->VInitialize();
		}

		template <class TScene>
		void LoadScene()
		{
			mSceneToLoad = new TScene();
		}

		bool unload;
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