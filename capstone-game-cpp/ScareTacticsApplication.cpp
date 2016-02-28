#include "stdafx.h"
#include "Rig3D/Engine.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"

//Shaders - Headers are output from compiler
#include "Shaders/obj/CSGridPass1.h"
#include "Shaders/obj/CSGridPass2.h"
#include "Shaders/obj/PSDef2DTexture.h"
#include "Shaders/obj/PSDefColor.h"
#include "Shaders/obj/PSDefMaterial.h"
#include "Shaders/obj/PSFwd2DTexture.h"
#include "Shaders/obj/PSFwdColor.h"
#include "Shaders/obj/PSFwdDeferredOutput.h"
#include "Shaders/obj/PSFwdSpotLightVolume.h"
#include "Shaders/obj/VSDefInstancedMaterial.h"
#include "Shaders/obj/VSDefSingleColor.h"
#include "Shaders/obj/VSDefSingleMaterial.h"
#include "Shaders/obj/VSDefSkinnedMaterial.h"
#include "Shaders/obj/VSFwdFullScreenQuad.h"
#include "Shaders/obj/VSFwdInstancedColor.h"
#include "Shaders/obj/VSFwdLineTrace.h"
#include "Shaders/obj/VSFwdSingleColor.h"
#include "Shaders/obj/VSFwdSpotLightVolume.h"
#include "Shaders/obj/VSFwdSprites.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace Rig3D;

ScareTacticsApplication::ScareTacticsApplication() :
	mCSGridPass1(nullptr),
	mCSGridPass2(nullptr),
	mPSDefColor(nullptr),
	mPSDefMaterial(nullptr),
	mPSFwd2DTexture(nullptr),
	mPSFwdColor(nullptr),
	mPSFwdDeferredOutput(nullptr),
	mPSFwdSpotLightVolume(nullptr),
	mPSDef2DTexture(nullptr),
	mVSDefInstancedMaterial(nullptr),
	mVSDefSingleColor(nullptr),
	mVSDefSingleMaterial(nullptr),
	mVSDefSkinnedMaterial(nullptr),
	mVSFwdFullScreenQuad(nullptr),
	mVSFwdInstancedColor(nullptr),
	mVSFwdSingleColor(nullptr),
	mVSFwdSpotLightVolume(nullptr),
	mVSFwdSprites(nullptr),
	mStudio(nullptr),
	mLoadingScreen(nullptr),
	mCurrentScene(nullptr),
	mSceneToLoad(nullptr),
	mSceneAllocator(),
	mStaticMemory(nullptr),
	mStaticMemorySize(0),
	mAcumTimer(0.0f)
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("Assets/Wil2ghan.ttf", 30.0f, NULL, NULL);
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
	mSceneAllocator.SetMemory(start, mStaticMemory + STATIC_SCENE_MEMORY + SCENE_ALIGNMENT_PADDING);				// Extra padding for alignment
	mGameAllocator.SetMemory(mStaticMemory + STATIC_SCENE_MEMORY + SCENE_ALIGNMENT_PADDING, mStaticMemory + size);	
	mModelManager.SetAllocator(&mGameAllocator);
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

void ScareTacticsApplication::InitializeShaders()
{
	auto engine = &Singleton<Engine>::SharedInstance();
	auto renderer = engine->GetRenderer();

#pragma region Vertex Shaders

	// Instancing
	InputElement instancingInput[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX   },
		{ "NORMAL",		0, 0, 12,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24,  0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0,  1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE }
	};
	renderer->VCreateShader(&mVSDefInstancedMaterial, &mGameAllocator);
	renderer->VLoadVertexShader(mVSDefInstancedMaterial, gVSDefInstancedMaterial, sizeof(gVSDefInstancedMaterial), instancingInput, 7);
	renderer->VCreateShader(&mVSFwdInstancedColor, &mGameAllocator);
	renderer->VLoadVertexShader(mVSFwdInstancedColor, gVSFwdInstancedColor, sizeof(gVSFwdInstancedColor), instancingInput, 7);

	// Single Mesh
	InputElement vertex3Input[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24,  0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX }
	};
	renderer->VCreateShader(&mVSDefSingleMaterial, &mGameAllocator);
	renderer->VLoadVertexShader(mVSDefSingleMaterial, gVSDefSingleMaterial, sizeof(gVSDefSingleMaterial), vertex3Input, 3);
	renderer->VCreateShader(&mVSDefSingleColor, &mGameAllocator);
	renderer->VLoadVertexShader(mVSDefSingleColor, gVSDefSingleColor, sizeof(gVSDefSingleColor), vertex3Input, 3);
	renderer->VCreateShader(&mVSFwdSingleColor, &mGameAllocator);
	renderer->VLoadVertexShader(mVSFwdSingleColor, gVSFwdSingleColor, sizeof(gVSFwdSingleColor), vertex3Input, 3);

	// Point Light
	InputElement plvInputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX }
	};
	renderer->VCreateShader(&mVSFwdSpotLightVolume, &mGameAllocator);
	renderer->VLoadVertexShader(mVSFwdSpotLightVolume, gVSFwdSpotLightVolume, sizeof(gVSFwdSpotLightVolume), plvInputElements, 1);

	// Full screen quad
	// No input :D Use Draw(3, 0) with no mesh bound.
	renderer->VCreateShader(&mVSFwdFullScreenQuad, &mGameAllocator);
	renderer->VLoadVertexShader(mVSFwdFullScreenQuad, gVSFwdFullScreenQuad, sizeof(gVSFwdFullScreenQuad), nullptr, 0);
	
	// Sprite
	InputElement spriteInputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 12, 0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "POINTPOS",	0, 1, 0,  1, RGB_FLOAT32,  INPUT_CLASS_PER_INSTANCE },
		{ "SIZE",		0, 1, 12, 1, RG_FLOAT32,  INPUT_CLASS_PER_INSTANCE },
		{ "SCALE",		0, 1, 20, 1, RG_FLOAT32,  INPUT_CLASS_PER_INSTANCE },
		{ "TEXID",		0, 1, 28, 1, R_FLOAT32,  INPUT_CLASS_PER_INSTANCE }
	};
	renderer->VCreateShader(&mVSFwdSprites, &mGameAllocator);
	renderer->VLoadVertexShader(mVSFwdSprites, gVSFwdSprites, sizeof(gVSFwdSprites), spriteInputElements, 6);

	// Skinned Vertex
	InputElement skinnedInputElements[] =
	{
		{ "BLENDINDICES",	0, 0, 0,	0,	RGBA_UINT32,	INPUT_CLASS_PER_VERTEX },
		{ "BLENDWEIGHTS",	0, 0, 16,	0,	RGBA_FLOAT32,	INPUT_CLASS_PER_VERTEX },
		{ "POSITION",		0, 0, 32,	0,	RGB_FLOAT32,	INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",			0, 0, 44,	0,	RGB_FLOAT32,	INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",		0, 0, 56,	0,	RG_FLOAT32,		INPUT_CLASS_PER_VERTEX }
	};
	renderer->VCreateShader(&mVSDefSkinnedMaterial, &mGameAllocator);
	renderer->VLoadVertexShader(mVSDefSkinnedMaterial, gVSDefSkinnedMaterial, sizeof(gVSDefSkinnedMaterial), skinnedInputElements, 5);

#pragma endregion

#pragma region Pixel Shaders

	renderer->VCreateShader(&mPSFwdDeferredOutput, &mGameAllocator);
	renderer->VLoadPixelShader(mPSFwdDeferredOutput, gPSFwdDeferredOutput, sizeof(gPSFwdDeferredOutput));

	renderer->VCreateShader(&mPSFwd2DTexture, &mGameAllocator);
	renderer->VLoadPixelShader(mPSFwd2DTexture, gPSFwd2DTexture, sizeof(gPSFwd2DTexture));

	renderer->VCreateShader(&mPSFwdSpotLightVolume, &mGameAllocator);
	renderer->VLoadPixelShader(mPSFwdSpotLightVolume, gPSFwdSpotLightVolume, sizeof(gPSFwdSpotLightVolume));

	renderer->VCreateShader(&mPSDefMaterial, &mGameAllocator);
	renderer->VLoadPixelShader(mPSDefMaterial, gPSDefMaterial, sizeof(gPSDefMaterial));

	renderer->VCreateShader(&mPSDefColor, &mGameAllocator);
	renderer->VLoadPixelShader(mPSDefColor, gPSDefColor, sizeof(gPSDefColor));

	renderer->VCreateShader(&mPSFwdColor, &mGameAllocator);
	renderer->VLoadPixelShader(mPSFwdColor, gPSFwdColor, sizeof(gPSFwdColor));

	renderer->VCreateShader(&mPSDef2DTexture, &mGameAllocator);
	renderer->VLoadPixelShader(mPSDef2DTexture, gPSDef2DTexture, sizeof(gPSDef2DTexture));

#pragma endregion

#pragma region Compute Shaders

	renderer->VCreateShader(&mCSGridPass1, &mGameAllocator);
	renderer->VLoadComputeShader(mCSGridPass1, gCSGridPass1, sizeof(gCSGridPass1));
	renderer->VCreateShader(&mCSGridPass2, &mGameAllocator);
	renderer->VLoadComputeShader(mCSGridPass2, gCSGridPass2, sizeof(gCSGridPass2));

#pragma endregion
}

void ScareTacticsApplication::InitializeFMOD()
{
	FMOD_CHECK(FMOD::Studio::System::create(&mStudio));
	FMOD_CHECK(mStudio->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr));

	// can be async with: FMOD_STUDIO_LOAD_BANK_NONBLOCKING
	FMOD::Studio::Bank* bank = nullptr;
	FMOD_CHECK(mStudio->loadBankFile("Assets/FMOD/Desktop/Master Bank.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
	FMOD_CHECK(mStudio->loadBankFile("Assets/FMOD/Desktop/Master Bank.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
	
	FMOD_CHECK(mStudio->loadBankFile("Assets/FMOD/Desktop/Minions.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
}

void ScareTacticsApplication::VInitialize()
{
	InitializeShaders();
	InitializeFMOD();

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

void ScareTacticsApplication::UpdateGroundMousePosition()
{
	auto mousePosition = mCurrentScene->mInput->mousePosition;
	mGroundMousePosition = mCurrentScene->mCameraManager->Screen2WorldAtZ0(mousePosition);
}

void ScareTacticsApplication::VUpdate(float deltaTime)
{
	mAcumTimer += deltaTime;

	// Update audio
	FMOD_CHECK(mStudio->update());

	if (mCurrentScene)
	{
		UpdateGroundMousePosition();
		mCurrentScene->VUpdate(deltaTime);
		if (mAcumTimer > 1000.0f / FIXED_UPDATE) {
			mCurrentScene->VFixedUpdate(mAcumTimer);
			mAcumTimer = 0;
		}
		mCurrentScene->VRender();
	}
	else
	{
		mLoadingScreen->VUpdate(deltaTime);
		if (mAcumTimer > 1000.0f / FIXED_UPDATE) {
			mLoadingScreen->VFixedUpdate(mAcumTimer);
			mAcumTimer = 0;
		}
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

	mCSGridPass1->~IShader();
	mCSGridPass2->~IShader();
	mPSDefColor->~IShader();
	mPSDefMaterial->~IShader();
	mPSFwd2DTexture->~IShader();
	mPSFwdColor->~IShader();
	mPSFwdDeferredOutput->~IShader();
	mPSFwdSpotLightVolume->~IShader();
	mPSDef2DTexture->~IShader();

	mVSDefInstancedMaterial->~IShader();
	mVSDefSingleColor->~IShader();
	mVSDefSingleMaterial->~IShader();
	mVSDefSkinnedMaterial->~IShader();
	mVSFwdFullScreenQuad->~IShader();
	mVSFwdInstancedColor->~IShader();
	mVSFwdSingleColor->~IShader();
	mVSFwdSpotLightVolume->~IShader();
	mVSFwdSprites->~IShader();

	mSceneAllocator.Free();
	mGameAllocator.Free();

	FMOD_CHECK(mStudio->release());
}