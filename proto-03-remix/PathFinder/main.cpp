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

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

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
	IMesh*							mLightMesh;
	IMesh*							mPlayerMesh;

#pragma region IScene Override
	Proto_03_Remix() : 
		mSceneAllocator((void*)gSceneMemory, (void*)(gSceneMemory + gSceneMemorySize)), 
		mStaticMeshAllocator((void*)gStaticMeshMemory, (void*)(gStaticMeshMemory + gMeshMemorySize)),
		mDynamicMeshAllocator((void*)gDynamicMeshMemory, (void*)(gDynamicMeshMemory + gMeshMemorySize))

	{
		mOptions.mWindowCaption = "Shutter - Remix";
		mOptions.mWindowWidth = 800;
		mOptions.mWindowHeight = 600;
		mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen = false;

		mStaticMeshLibrary.SetAllocator(&mStaticMeshAllocator);
		mDynamicMeshLibrary.SetAllocator(&mDynamicMeshAllocator);
	}

	~Proto_03_Remix()
	{
		
	}

	 void VInitialize() override {
		 auto graph = PathFinder::Graph<10, 10>();
	 }
	void VUpdate(double milliseconds) override {

	}


	void VRender() override {}
	void VShutdown() override{}
	void VOnResize() override{}
#pragma endregion

#pragma region Initialization
	void LoadWalls(vec3f* positions, vec3f* rotations, vec3f* scales, int size)
	{
		// Allocate size SceneObjects
		mWallCount = size;
		mWalls = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject) * size, alignof(SceneObject), 0));
		for (int i = 0; i < size; i++)
		{
			mWalls[i].mTransform.mPosition	= positions[i];
			mWalls[i].mTransform.mRotation	= rotations[i];
			mWalls[i].mTransform.mScale		= scales[i];
		}
	}
#pragma endregion 
};

DECLARE_MAIN(Proto_03_Remix);