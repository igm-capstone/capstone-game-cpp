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
#include "Fringe.h"
#include "SceneObject.h"
#include "LevelReader.h"
#include "Rig3D/GraphicsMath/cgm.h"
#include "Vertex.h"

#define PI 3.1415926535f

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

static const vec4f gWallColor = { 1.0f, 0.0f, 1.0f, 1.0f };

static const int gCircleVertexCount = 13;
static const int gCircleIndexCount	= 36;	// Indices = (vertices - 1) * 3

static const int gSceneMemorySize	= 20480; 
static const int gMeshMemorySize	= 1024;

char gSceneMemory[gSceneMemorySize];
char gStaticMeshMemory[gMeshMemorySize];
char gDynamicMeshMemory[gMeshMemorySize];


class Proto_03_Remix : public IScene, public virtual IRendererDelegate
{
	PathFinder::Graph<10, 10> graph;

public:

	struct QuadShaderData
	{
		mat4f View;
		mat4f Projection;
		vec4f Color;
	};

	QuadShaderData					mQuadShaderData;

	mat4f							mViewMatrix;
	mat4f							mProjectionMatrix;

	LinearAllocator					mSceneAllocator;
	LinearAllocator					mStaticMeshAllocator;
	LinearAllocator					mDynamicMeshAllocator; // TO DO: This will likely need to be a Pool Allocator ->Gabe

	MeshLibrary<LinearAllocator>	mStaticMeshLibrary;
	MeshLibrary<LinearAllocator>	mDynamicMeshLibrary;

	SceneObject*					mWalls;
	SceneObject*					mBlocks;
	SceneObject*					mRobots;
	SceneObject*					mLights;
	SceneObject*					mPlayer;
	SceneObject*					mGoal;

	mat4f*							mWallTransforms;
	mat4f*							mBlockTransforms;
	mat4f*							mLightTransforms;

	int								mWallCount;
	int								mBlockCount;
	int								mLightCount;

	IMesh*							mWallMesh;
	IMesh*							mRobotMesh;
	IMesh*							mCircleMesh;
	IMesh*							mLightMesh;
	IMesh*							mPlayerMesh;

	DX3D11Renderer*					mRenderer;
	Input*							mInput;

	ID3D11Device*					mDevice;
	ID3D11DeviceContext*			mDeviceContext;

	ID3D11InputLayout*				mQuadInputLayout;
	ID3D11VertexShader*				mQuadVertexShader;
	ID3D11PixelShader*				mQuadPixelShader;
	ID3D11Buffer*					mQuadShaderBuffer;
	ID3D11Buffer*					mWallInstanceBuffer;

#pragma region IScene Override
	Proto_03_Remix() :
		mSceneAllocator((void*)gSceneMemory, (void*)(gSceneMemory + gSceneMemorySize)),
		mStaticMeshAllocator((void*)gStaticMeshMemory, (void*)(gStaticMeshMemory + gMeshMemorySize)),
		mDynamicMeshAllocator((void*)gDynamicMeshMemory, (void*)(gDynamicMeshMemory + gMeshMemorySize))

	{
		mOptions.mWindowCaption = "Shutter - Remix";
		mOptions.mWindowWidth	= 1600;
		mOptions.mWindowHeight	= 1000;
		mOptions.mGraphicsAPI	= GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen	= false;
		mStaticMeshLibrary.SetAllocator(&mStaticMeshAllocator);
		mDynamicMeshLibrary.SetAllocator(&mDynamicMeshAllocator);
	}

	~Proto_03_Remix() {}

	void VInitialize() override
	{
		mRenderer = &DX3D11Renderer::SharedInstance();
		mDeviceContext = mRenderer->GetDeviceContext();
		mDevice = mRenderer->GetDevice();

		mInput = &Input::SharedInstance();

		InitializeLevel();
		InitializeGeometry();
		InitializeShaders();
		InitializeCamera();

		// TO DO: Make Initialize function (InitializeGraph)
		graph = PathFinder::Graph<10, 10>();
		graph.grid[4][8].weight = 100;
		graph.grid[4][7].weight = 100;
		graph.grid[4][6].weight = 100;
		graph.grid[4][5].weight = 100;
		graph.grid[4][4].weight = 100;
		graph.grid[4][3].weight = 100;


		
	}
	void VUpdate(double milliseconds) override 
	{
		if ((&Input::SharedInstance())->GetKeyDown(KEYCODE_UP))
		{
			TRACE("Treta" << 1 << " " << 1.0f << true);

			auto search = PathFinder::Fringe<10, 10>(graph);

			auto start = &graph.grid[1][5];
			auto end = &graph.grid[8][5];

			auto result = search.FindPath(start, end);

			std::stringstream ss;
			for (int y = 0; y < 10; y++)
			{
				for (int x = 0; x < 10; x++)
				{
					bool inPath = false;
					for (auto it = result.path.begin(); it != result.path.end(); ++it)
					{
						if (**it == graph.grid[x][y])
						{
							inPath = true;
							break;
						}
					}

					ss << " " << (inPath ? 'X' : graph.grid[x][y].weight > 1 ? '#' : 'O');
				}
				ss << std::endl;
			}

			TRACE(ss.str());

		}
	}

	void VRender() override
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);

		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
		mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		RenderWalls();

		mRenderer->VSwapBuffers();
	}

	void VShutdown() override {
		mCircleMesh->~IMesh();
		mWallMesh->~IMesh();
		
		ReleaseMacro(mQuadInputLayout);
		ReleaseMacro(mQuadVertexShader);
		ReleaseMacro(mQuadPixelShader);
		ReleaseMacro(mQuadShaderBuffer);
		ReleaseMacro(mWallInstanceBuffer);
	}
	void VOnResize() override {}
#pragma endregion

#pragma region Render
	void RenderWalls()
	{
		const UINT stride = sizeof(mat4f);
		const UINT offset = 0;
		mDeviceContext->IASetInputLayout(mQuadInputLayout);
		mDeviceContext->VSSetShader(mQuadVertexShader, NULL, 0);
		mDeviceContext->PSSetShader(mQuadPixelShader, NULL, 0);

		mDeviceContext->UpdateSubresource(mQuadShaderBuffer, 0, NULL, &mQuadShaderData, 0, 0);
		mDeviceContext->VSSetConstantBuffers(0, 1, &mQuadShaderBuffer);

		mRenderer->VBindMesh(mWallMesh);
		mDeviceContext->IASetVertexBuffers(1, 1, &mWallInstanceBuffer, &stride, &offset);

		mDeviceContext->DrawIndexedInstanced(mWallMesh->GetIndexCount(), mWallCount, 0, 0, 0);
	}
#pragma endregion 

#pragma region Initialization
	void InitializeCamera()
	{
		float aspectRatio = (float)mOptions.mWindowWidth / mOptions.mWindowHeight;
		float halfHeight = 33.5f;
		float halfWidth = 33.5f * aspectRatio;
		mProjectionMatrix = mat4f::normalizedOrthographicLH(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.3f, 1000.0f).transpose();
		mViewMatrix = mat4f::lookAtLH(vec3f(0.0f, 0.0f, 1.0f), vec3f(0.0f, 0.0f, -30.0f), vec3f(0.0f, 1.0f, 0.0f)).transpose();

		mQuadShaderData.View = mViewMatrix;
		mQuadShaderData.Projection = mProjectionMatrix;
		mQuadShaderData.Color = gWallColor;
	}

	void InitializeLevel()
	{
		LevelReader levelReader("UnitySceneExport.json");
		levelReader.ReadLevel();

		// Walls
		LoadTransforms(&mWallTransforms, &levelReader.mWalls.Position[0], &levelReader.mWalls.Rotation[0], &levelReader.mWalls.Scale[0], levelReader.mWalls.Position.size(), 0);
		mWallCount = levelReader.mWalls.Position.size();

		// Blocks
		LoadTransforms(&mBlockTransforms, &levelReader.mBlocks.Position[0], &levelReader.mBlocks.Rotation[0], &levelReader.mBlocks.Scale[0], levelReader.mBlocks.Position.size(), 0);
		mBlockCount = levelReader.mBlocks.Position.size();

		// Lights
		LoadTransforms(&mLightTransforms, &levelReader.mLights[0], NULL, NULL, levelReader.mLights.size(), 1);
		mLightCount = levelReader.mLights.size();

		// Player
		mPlayer = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject), alignof(SceneObject), 0));
		mPlayer->mTransform.mPosition = levelReader.mPlayerPos;

		// Goal
		mGoal = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject), alignof(SceneObject), 0));
		mGoal->mTransform.mPosition = levelReader.mGoalPos;
	}

	void LoadTransforms(mat4f** transforms, vec3f* positions, vec3f* rotations, vec3f* scales, int size, int TransformType)
	{
		*transforms = reinterpret_cast<mat4f*>(mSceneAllocator.Allocate(sizeof(mat4f) * size, alignof(mat4f), 0));

		switch (TransformType)
		{
		case 0:
		{
			for (int i = 0; i < size; i++)
			{
				(*transforms)[i] = (mat4f::scale(scales[i]) * mat4f::rotateY(PI) * mat4f::translate(positions[i])).transpose();
			}
			break;
		}
		case 1:
		{
			for (int i = 0; i < size; i++)
			{
				(*transforms)[i] = mat4f::translate(positions[i]).transpose();
			}
			break;
		}
		default:
			break;
		}
	}

	void LoadSceneObjectData(SceneObject** sceneObjects, vec3f* positions, vec3f* rotations, vec3f* scales, int size, int TransformType)
	{
		// Allocate size SceneObjects
		*sceneObjects = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject) * size, alignof(SceneObject), 0));

		switch(TransformType)
		{
		case 0:
		{
			for (int i = 0; i < size; i++)
			{
				(*sceneObjects)[i].mTransform.mPosition = positions[i];
				(*sceneObjects)[i].mTransform.mRotation = rotations[i];
				(*sceneObjects)[i].mTransform.mScale = scales[i];
			}
			break;
		}
		case 1:
		{
			for (int i = 0; i < size; i++)
			{
				(*sceneObjects)[i].mTransform.mPosition = positions[i];
			}
			break;
		}
		default:
			break;
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
			{ -0.85f, -0.85f, 0.0f },
			{ +0.85f, -0.85f, 0.0f },
			{ +0.85f, +0.85f, 0.0f },
			{ -0.85f, +0.85f, 0.0f }
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

	void InitializeShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"QuadVertexShader.cso", &vsBlob);

		// Create the shader on the device
		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&mQuadVertexShader);

		// Before cleaning up the data, create the input layout
		if (inputDescription) {
			mDevice->CreateInputLayout(
				inputDescription,					// Reference to Description
				5,									// Number of elments inside of Description
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mQuadInputLayout);
		}

		// Clean up
		vsBlob->Release();

		// Load Pixel Shader ---------------------------------------
		ID3DBlob* psBlob;
		D3DReadFileToBlob(L"QuadPixelShader.cso", &psBlob);

		// Create the shader on the device
		mDevice->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			NULL,
			&mQuadPixelShader);

		// Clean up
		psBlob->Release();

		// Instance buffer
		D3D11_BUFFER_DESC quadInstanceBufferDesc;
		quadInstanceBufferDesc.ByteWidth = sizeof(mat4f) * mWallCount;
		quadInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		quadInstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		quadInstanceBufferDesc.CPUAccessFlags = 0;
		quadInstanceBufferDesc.MiscFlags = 0;
		quadInstanceBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA instanceData;
		instanceData.pSysMem = mWallTransforms;

		mDevice->CreateBuffer(&quadInstanceBufferDesc, &instanceData, &mWallInstanceBuffer);

		// Constant buffers ----------------------------------------
		D3D11_BUFFER_DESC quadBufferDataDesc;
		quadBufferDataDesc.ByteWidth = sizeof(QuadShaderData);
		quadBufferDataDesc.Usage = D3D11_USAGE_DEFAULT;
		quadBufferDataDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		quadBufferDataDesc.CPUAccessFlags = 0;
		quadBufferDataDesc.MiscFlags = 0;
		quadBufferDataDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&quadBufferDataDesc, NULL, &mQuadShaderBuffer);
	}

#pragma endregion 
};

DECLARE_MAIN(Proto_03_Remix);