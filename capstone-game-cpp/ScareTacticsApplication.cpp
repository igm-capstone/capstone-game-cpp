#include "stdafx.h"
#include "Rig3D/Engine.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"

//Shaders - Headers are output from compiler
#include "Shaders/obj/BillboardPixelShader.h"
#include "Shaders/obj/BillboardVertexShader.h"
#include "Shaders/obj/CircleVertexShader.h"
#include "Shaders/obj/QuadPixelShader.h"
#include "Shaders/obj/QuadVertexShader.h"
#include "Shaders/obj/ShadowCasterPixelShader.h"
#include "Shaders/obj/ShadowGridComputeShader.h"
#include "Shaders/obj/ShadowPixelShader.h"

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
	mSceneAllocator.SetMemory(start, mStaticMemory + STATIC_SCENE_MEMORY + 6);	// Extra padding for alignment
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
	auto device = renderer->GetDevice();

	InputElement inputElements[] =
	{
		{ "POSITION", 0, 0, 0,  0, RGB_FLOAT32,  INPUT_CLASS_PER_VERTEX,   },
		{ "WORLD",    0, 1, 0,  1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE, },
		{ "WORLD",    1, 1, 16, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE, },
		{ "WORLD",    2, 1, 32, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE, },
		{ "WORLD",    3, 1, 48, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE, },
	};

	IShader* vertexShader;
	renderer->VCreateShader(&vertexShader, &mGameAllocator);

	// TODO create overload to replace block
	// renderer->VLoadPixelShader(vertexShader, gQuadVertexShader, sizeof(gQuadVertexShader), inputElements, 5);
	{
		auto dxVertexShader = static_cast<DX11Shader*>(vertexShader);

		device->CreateVertexShader(gQuadVertexShader, sizeof(gQuadVertexShader), nullptr,
			&dxVertexShader->mVertexShader);

		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		device->CreateInputLayout(inputDescription, 5, gQuadVertexShader,
			sizeof(gQuadVertexShader), &dxVertexShader->mInputLayout);
	}
	mShaders["QuadVertex"] = vertexShader;

	IShader* pixelShader;
	renderer->VCreateShader(&pixelShader, &mGameAllocator);

	// TODO create overload to replace block
	// renderer->VLoadPixelShader(pixelShader, gQuadPixelShader, sizeof(gQuadPixelShader));
	{
		device->CreatePixelShader(gQuadPixelShader, sizeof(gQuadPixelShader), nullptr,
			&static_cast<DX11Shader*>(pixelShader)->mPixelShader);
	}
	mShaders["QuadPixel"] = pixelShader;
}

void ScareTacticsApplication::VInitialize()
{
	auto memory = static_cast<char*>(malloc(sizeof(char) * 1000));
	mGameAllocator.SetMemory(memory, memory + 1000);

	Renderer* renderer = Singleton<Engine>::SharedInstance().GetRenderer();

	InitializeShaders();

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