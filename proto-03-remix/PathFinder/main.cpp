#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\LinearAllocator.h"
#include "Rig3D\MeshLibrary.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include "PathFinder\Graph.h"
#include "SceneObject.h"

#define PI 3.1415926535f

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

static const int gCircleVertexCount = 13;
static const int gCircleIndexCount	= 36;	// Indices = (vertices - 1) * 3

static const int gSceneMemorySize	= 1024;
static const int gMeshMemorySize	= 1024;

static const char gSceneMemory[gSceneMemorySize];
static const char gStaticMeshMemory[gMeshMemorySize];
static const char gDynamicMeshMemory[gMeshMemorySize];


class Proto_03_Remix : public IScene, public virtual IRendererDelegate
{
public:

	LinearAllocator					mSceneAllocator;
	LinearAllocator					mStaticMeshAllocator;
	LinearAllocator					mDynamicMeshAllocator; // TO DO: This will likely need to be a Pool Allocator ->Gabe

	MeshLibrary<LinearAllocator>	mStaticMeshLibrary;
	MeshLibrary<LinearAllocator>	mDynamicMeshLibrary;

	SceneObject*					mWalls;
	SceneObject*					mRobots;
	SceneObject*					mLights;
	SceneObject*					mPlayer;

	int								mWallCount;

	IMesh*							mWallMesh;
	IMesh*							mRobotMesh;
	IMesh*							mCircleMesh;
	IMesh*							mLightMesh;
	IMesh*							mPlayerMesh;

	DX3D11Renderer*					mRenderer;
	Input*							mInput;

#pragma region IScene Override
	Proto_03_Remix() :
		mSceneAllocator((void*)gSceneMemory, (void*)(gSceneMemory + gSceneMemorySize)),
		mStaticMeshAllocator((void*)gStaticMeshMemory, (void*)(gStaticMeshMemory + gMeshMemorySize)),
		mDynamicMeshAllocator((void*)gDynamicMeshMemory, (void*)(gDynamicMeshMemory + gMeshMemorySize))

	{
		mOptions.mWindowCaption = "Shutter - Remix";
		mOptions.mWindowWidth	= 800;
		mOptions.mWindowHeight	= 600;
		mOptions.mGraphicsAPI	= GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen	= false;

		mStaticMeshLibrary.SetAllocator(&mStaticMeshAllocator);
		mDynamicMeshLibrary.SetAllocator(&mDynamicMeshAllocator);
	}

	~Proto_03_Remix()
	{

	}

	void VInitialize() override 
	{
		mRenderer	= &DX3D11Renderer::SharedInstance();
		mInput		= &Input::SharedInstance();

		auto graph = PathFinder::Graph<10, 10>();


	}
	void VUpdate(double milliseconds) override {

	}


	void VRender() override {}
	void VShutdown() override {}
	void VOnResize() override {}
#pragma endregion

#pragma region Initialization
	// Used for walls and blocks... Write overloads for sceneobjects with additional data (robots)
	void LoadSceneObjectData(SceneObject* sceneObjects, vec3f* positions, vec3f* rotations, vec3f* scales, int size)
	{
		// Allocate size SceneObjects
		sceneObjects = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject) * size, alignof(SceneObject), 0));
		for (int i = 0; i < size; i++)
		{
			sceneObjects[i].mTransform.mPosition	= positions[i];
			sceneObjects[i].mTransform.mRotation	= rotations[i];
			sceneObjects[i].mTransform.mScale		= scales[i];
		}
	}

	void InitializeGeometry()
	{
		InitializeQuadMesh();
		InitializeCircleMesh();
	}

	void InitializeQuadMesh()
	{
		vec3f quadVertices[4] =
		{
			{ -1.0f, -1.0f, 0.0f },
			{ +1.0f, -1.0f, 0.0f },
			{ +1.0f, +1.0f, 0.0f },
			{ -1.0f, +1.0f, 0.0f }
		};

		uint16_t quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

		mStaticMeshLibrary.NewMesh(&mWallMesh, mRenderer);
		mRenderer->VSetMeshVertexBufferData(mWallMesh, quadVertices, sizeof(vec3f) * 4, sizeof(vec3f), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mWallMesh, quadIndices, 6, GPU_MEMORY_USAGE_STATIC);
	}

	void InitializeCircleMesh()
	{
		vec3f circleVertices[gCircleVertexCount];
		float angularDisplacement = (2.0f * PI) / gCircleVertexCount;
		float radius	= 1.0f;
		float angle		= 0.0f;

		circleVertices[0] = { 0.0f, 0.0f, 0.0f };
		for (int i = 1; i < gCircleVertexCount; i++, angle += angularDisplacement)
		{
			circleVertices[i] = { radius * cosf(angle), radius * sinf(angle), 0.0f };
		}

		uint16_t circleIndices[gCircleIndexCount] =
		{
			0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
			0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 9,
			0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 1
		};

		mStaticMeshLibrary.NewMesh(&mCircleMesh, mRenderer);
		mRenderer->VSetMeshVertexBufferData(mCircleMesh, circleVertices, sizeof(vec3f) * gCircleVertexCount, sizeof(vec3f), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mCircleMesh, circleIndices, gCircleIndexCount, GPU_MEMORY_USAGE_STATIC);
	}
#pragma endregion 
};

DECLARE_MAIN(Proto_03_Remix);