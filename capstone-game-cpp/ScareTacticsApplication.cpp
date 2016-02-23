#include "stdafx.h"
#include "Rig3D/Engine.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Rig3D/Graphics/DirectX11/DX11Shader.h"

//Shaders - Headers are output from compiler
#include "Shaders/obj/BillboardPixelShader.h"
#include "Shaders/obj/BillboardVertexShader.h"
#include "Shaders/obj/CircleVertexShader.h"
#include "Shaders/obj/StaticMeshPixelShader.h"
#include "Shaders/obj/StaticMeshVertexShader.h"
#include "Shaders/obj/ExplorerVertexShader.h"
#include "Shaders/obj/ExplorerPixelShader.h"
#include "Shaders/obj/SpotLightVolumeVertexShader.h"
#include "Shaders/obj/SpotLightVolumePixelShader.h"
#include "Shaders/obj/NDSQuadVertexShader.h"
#include "Shaders/obj/NDSQuadPixelShader.h"
#include "Shaders/obj/DebugTexturePixelShader.h"
#include "Shaders/obj/SpriteVertexShader.h"
#include "Shaders/obj/SpritePixelShader.h"
#include "Shaders/obj/ShadowCasterPixelShader.h"
#include "Shaders/obj/GridPass1ComputeShader.h"
#include "Shaders/obj/GridPass2ComputeShader.h"
#include "Shaders/obj/ShadowPixelShader.h"
#include "Shaders/obj/SkinnedMeshVertexShader.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>
#include <Rig3D/Intersection.h>

using namespace Rig3D;

ScareTacticsApplication::ScareTacticsApplication() :
	mStaticMeshVertexShader(nullptr),
	mStaticMeshPixelShader(nullptr),
	mExplorerVertexShader(nullptr),
	mExplorerPixelShader(nullptr),
	mPLVolumeVertexShader(nullptr),
	mPLVolumePixelShader(nullptr),
	mNDSQuadVertexShader(nullptr),
	mNDSQuadPixelShader(nullptr),
	mSpriteVertexShader(nullptr),
	mSpritePixelShader(nullptr),
	mSkinnedVertexShader(nullptr),
	mGridPass1ComputeShader(nullptr),
	mGridPass2ComputeShader(nullptr),
	mDBGPixelShader(nullptr),
	mStudio(nullptr),
	mLoadingScreen(nullptr),
	mCurrentScene(nullptr),
	mSceneToLoad(nullptr),
	mSceneAllocator(),
	mStaticMemory(nullptr),
	mStaticMemorySize(0)
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

	// Static Mesh shaders

	InputElement staticMeshInputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX   },
		{ "NORMAL",		0, 0, 12,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24,  0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0,  1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE }
	};

	renderer->VCreateShader(&mStaticMeshVertexShader, &mGameAllocator);
	renderer->VCreateShader(&mStaticMeshPixelShader, &mGameAllocator);

	renderer->VLoadVertexShader(mStaticMeshVertexShader, gStaticMeshVertexShader, sizeof(gStaticMeshVertexShader), staticMeshInputElements, 7);
	renderer->VLoadPixelShader(mStaticMeshPixelShader, gStaticMeshPixelShader, sizeof(gStaticMeshPixelShader));

	// Explorer Shaders

	InputElement explorerInputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24,  0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX }
	};

	renderer->VCreateShader(&mExplorerVertexShader, &mGameAllocator);
	renderer->VCreateShader(&mExplorerPixelShader, &mGameAllocator);

	renderer->VLoadVertexShader(mExplorerVertexShader, gExplorerVertexShader, sizeof(gExplorerVertexShader), explorerInputElements, 3);
	renderer->VLoadPixelShader(mExplorerPixelShader, gExplorerPixelShader, sizeof(gExplorerPixelShader));

	// Point Light Shaders

	InputElement plvInputElements[] = 
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX }
	};

	renderer->VCreateShader(&mPLVolumeVertexShader, &mGameAllocator);
	renderer->VCreateShader(&mPLVolumePixelShader, &mGameAllocator);

	renderer->VLoadVertexShader(mPLVolumeVertexShader, gSpotLightVolumeVertexShader, sizeof(gSpotLightVolumeVertexShader), plvInputElements, 1);
	renderer->VLoadPixelShader(mPLVolumePixelShader, gSpotLightVolumePixelShader, sizeof(gSpotLightVolumePixelShader));

	// Normalized Device Quad Shaders

	InputElement ndsqInputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 12,  0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX }
	};

	renderer->VCreateShader(&mNDSQuadVertexShader, &mGameAllocator);
	renderer->VCreateShader(&mNDSQuadPixelShader, &mGameAllocator);

	renderer->VLoadVertexShader(mNDSQuadVertexShader, gNDSQuadVertexShader, sizeof(gNDSQuadVertexShader), ndsqInputElements, 2);
	renderer->VLoadPixelShader(mNDSQuadPixelShader, gNDSQuadPixelShader, sizeof(gNDSQuadPixelShader));

	renderer->VCreateShader(&mDBGPixelShader, &mGameAllocator);
	renderer->VLoadPixelShader(mDBGPixelShader, gDebugTexturePixelShader, sizeof(gDebugTexturePixelShader));

	// Sprite Shaders

	InputElement spriteInputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 12, 0, RG_FLOAT32,  INPUT_CLASS_PER_VERTEX },
		{ "POINTPOS",	0, 1, 0,  1, RGB_FLOAT32,  INPUT_CLASS_PER_INSTANCE },
		{ "SIZE",		0, 1, 12, 1, RG_FLOAT32,  INPUT_CLASS_PER_INSTANCE },
		{ "SCALE",		0, 1, 20, 1, RG_FLOAT32,  INPUT_CLASS_PER_INSTANCE },
		{ "TEXID",		0, 1, 28, 1, R_FLOAT32,  INPUT_CLASS_PER_INSTANCE }
	};

	renderer->VCreateShader(&mSpriteVertexShader, &mGameAllocator);
	renderer->VCreateShader(&mSpritePixelShader, &mGameAllocator);

	renderer->VLoadVertexShader(mSpriteVertexShader, gSpriteVertexShader, sizeof(gSpriteVertexShader), spriteInputElements, 6);
	renderer->VLoadPixelShader(mSpritePixelShader, gSpritePixelShader, sizeof(gSpritePixelShader));	
	
	// Skinned Vertex Shader

	InputElement skinnedInputElements[] =
	{
		{ "BLENDINDICES",	0, 0, 0,	0,	RGBA_UINT32,	INPUT_CLASS_PER_VERTEX },
		{ "BLENDWEIGHTS",	0, 0, 16,	0,	RGBA_FLOAT32,	INPUT_CLASS_PER_VERTEX },
		{ "POSITION",		0, 0, 32,	0,	RGB_FLOAT32,	INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",			0, 0, 44,	0,	RGB_FLOAT32,	INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",		0, 0, 56,	0,	RG_FLOAT32,		INPUT_CLASS_PER_VERTEX }
	};

	renderer->VCreateShader(&mSkinnedVertexShader, &mGameAllocator);

	renderer->VLoadVertexShader(mSkinnedVertexShader, gSkinnedMeshVertexShader, sizeof(gSkinnedMeshVertexShader), skinnedInputElements, 5);

	// Grid Compute Shader

	renderer->VCreateShader(&mGridPass1ComputeShader, &mGameAllocator);
	renderer->VLoadComputeShader(mGridPass1ComputeShader, gGridPass1ComputeShader, sizeof(gGridPass1ComputeShader));
	renderer->VCreateShader(&mGridPass2ComputeShader, &mGameAllocator);
	renderer->VLoadComputeShader(mGridPass2ComputeShader, gGridPass2ComputeShader, sizeof(gGridPass2ComputeShader));
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

	mStaticMeshVertexShader->~IShader();
	mStaticMeshPixelShader->~IShader();
	mExplorerVertexShader->~IShader();
	mExplorerPixelShader->~IShader();
	mPLVolumeVertexShader->~IShader();
	mPLVolumePixelShader->~IShader();
	mNDSQuadVertexShader->~IShader();
	mNDSQuadPixelShader->~IShader();
	mSpriteVertexShader->~IShader();
	mSpritePixelShader->~IShader();
	mDBGPixelShader->~IShader();
	mSkinnedVertexShader->~IShader();
	mGridPass1ComputeShader->~IShader();
	mGridPass2ComputeShader->~IShader();

	mSceneAllocator.Free();
	mGameAllocator.Free();

	FMOD_CHECK(mStudio->release());
}