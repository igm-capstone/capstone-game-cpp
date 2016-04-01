#pragma once
#include "Rig3D/IApplication.h"
#include "capstone-game-cpp/Scenes/BaseScene.h"
#include <Rig3D/Singleton.h>
#include <assert.h>
#include <Rig3D/Graphics/Interface/IShader.h>
#include "fmodwrap.h"
#include "ModelManager.h"

#define STATIC_SCENE_MEMORY		2000000
#define SCENE_ALIGNMENT_PADDING 6

using namespace Rig3D;

class ScareTacticsApplication : public IApplication
{
public:
	// Shaders

	IShader* mCSGridPass1;
	IShader* mCSGridPass2;
	IShader* mPSDefColor;
	IShader* mPSDefMaterial;
	IShader* mPSDefInstancedMaterial;
	IShader* mPSFwd2DTexture;
	IShader* mPSFwdColor;
	IShader* mPSFwdDeferredOutput;
	IShader* mPSFwdSpotLightVolume;
	IShader* mPSDef2DTexture;

	IShader* mVSDefInstancedColor;
	IShader* mVSDefInstancedMaterial;
	IShader* mVSDefSingleColor;
	IShader* mVSDefSingleMaterial;
	IShader* mVSDefSkinnedMaterial;
	IShader* mVSFwdFullScreenQuad;
	IShader* mVSFwdSingleColor;
	IShader* mVSFwdSpotLightVolume;
	IShader* mVSFwdSprites;

	FMOD::Studio::System* mStudio;
	vec3f mGroundMousePosition;
	ScareTacticsApplication();
	~ScareTacticsApplication();

	void SetLoadingScreen(BaseScene* loading);
	void SetStaticMemory(void* start, size_t size);

	void VInitialize() override;
	void VUpdateCurrentScene() override;
	void UpdateGroundMousePosition();
	void VUpdate(float deltaTime) override;
	void VShutdown() override;

	void InitializeShaders();
	void InitializeFMOD();

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
		
		mSceneToLoad = asScene;
	}

	void UnloadScene();
	BaseScene*	GetCurrentScene() const { return mCurrentScene; }
	ModelManager*	GetModelManager() { return &mModelManager; }

private:
	BaseScene*		mLoadingScreen;
	BaseScene*		mCurrentScene;
	BaseScene*		mSceneToLoad;

	LinearAllocator mGameAllocator;
	LinearAllocator mSceneAllocator;	// Not sure if this is the best guy for the job, but we will see.

	char*			mStaticMemory;
	size_t			mStaticMemorySize;

	ModelManager	mModelManager;

	double mAcumTimer;
};

typedef Singleton<ScareTacticsApplication> Application;
