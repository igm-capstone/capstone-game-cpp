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
#include "PathFinder/Fringe.h"
#include "SceneObject.h"
#include "LevelReader.h"
#include "Rig3D/GraphicsMath/cgm.h"
#include "Vertex.h"
#include "Grid.h"

#define PI 3.1415926535f

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

static const vec4f gWallColor = { 1.0f, 0.0f, 1.0f, 1.0f };
static const vec4f gCircleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
static const vec4f gPlayerColor = { 0.0f, 0.0f, 1.0f, 1.0f };
static const int gCircleVertexCount = 13;
static const int gCircleIndexCount	= 36;	// Indices = (vertices - 1) * 3

static const int gSceneMemorySize	= 20480; 
static const int gMeshMemorySize	= 1024;

char gSceneMemory[gSceneMemorySize];
char gStaticMeshMemory[gMeshMemorySize];
char gDynamicMeshMemory[gMeshMemorySize];


class Proto_03_Remix : public IScene, public virtual IRendererDelegate
{
	Grid& grid = grid.getInstance();

public:

	struct QuadShaderData
	{
		mat4f View;
		mat4f Projection;
		vec4f Color;
	};

	struct PointShaderData
	{
		vec4f Point;
	};

	QuadShaderData					mQuadShaderData;
	PointShaderData					mPointShaderData;

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
	mat4f*							mCircleTransforms;
	float*							mCircleColorWeights;
	mat4f							mPlayerTransform;
	std::vector<vec3f>				mLightPos;

	int								mWallCount;
	int								mBlockCount;
	int								mCircleCount;

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

	ID3D11InputLayout*				mCircleInputLayout;
	ID3D11VertexShader*				mCircleVertexShader;
	ID3D11PixelShader*				mCirclePixelShader;
	ID3D11Buffer*					mCircleInstanceBuffer;
	ID3D11Buffer*					mColorWeightInstanceBuffer;

	ID3D11Buffer*					mPlayerInstanceBuffer;

	ID3D11RenderTargetView*			mShadowCastersRTV;
	ID3D11Texture2D*				mShadowCastersMap;
	ID3D11ShaderResourceView*		mShadowCastersSRV;
	ID3D11RenderTargetView*			mShadowsARTV;
	ID3D11Texture2D*				mShadowsAMap;
	ID3D11ShaderResourceView*		mShadowsASRV;
	ID3D11RenderTargetView*			mShadowsBRTV;
	ID3D11Texture2D*				mShadowsBMap;
	ID3D11ShaderResourceView*		mShadowsBSRV;
	ID3D11RenderTargetView*			mShadowsFinalRTV;
	ID3D11Texture2D*				mShadowsFinalMap;
	ID3D11ShaderResourceView*		mShadowsFinalSRV;
	ID3D11PixelShader*				mShadowCasterPixelShader;
	ID3D11VertexShader*				mBillboardVertexShader;
	ID3D11PixelShader*				mBillboardPixelShader;
	ID3D11VertexShader*				mShadowVertexShader;
	ID3D11PixelShader*				mShadowPixelShader;
	ID3D11SamplerState*				mSamplerState;
	ID3D11BlendState*				mBlendStateShadowMask;
	ID3D11BlendState*				mBlendStateShadowCalc;
	ID3D11Buffer*					mPointShaderBuffer;
	
	float white[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float transp[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float black[4] = { 0.0f, 0.0f, 0.0f, 0.5f };

	ID3D11ShaderResourceView* nullSRV[2] = { 0, 0 };

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

		mRenderer->SetDelegate(this);
		VOnResize();

		InitializeLevel();
		InitializeGeometry();
		InitializeWallShaders();
		InitializeLightShaders();
		InitializePlayerShaders();
		InitializeCamera();

		// TO DO: Make Initialize function (InitializeGraph)
		/*graph = PathFinder::Graph<Node, 10, 10>();
		graph.grid[4][8].weight = 100;
		graph.grid[4][7].weight = 100;
		graph.grid[4][6].weight = 100;
		graph.grid[4][5].weight = 100;
		graph.grid[4][4].weight = 100;
		graph.grid[4][3].weight = 100;*/
	}

	void VUpdate(double milliseconds) override
	{
		float mPlayerSpeed = 0.25f;
		bool moved = false;

		if ((&Input::SharedInstance())->GetKeyDown(KEYCODE_F))
		{
			auto start = Vector3(10, 20, 0);
			auto end = Vector3(-20, -20, 0);
			grid.GetFringePath(start, end);
		}
		
		if ((&Input::SharedInstance())->GetKey(KEYCODE_LEFT))
		{
			mPlayer->mTransform.mPosition.x -= mPlayerSpeed;
			UpdatePlayer();
		}
		else if ((&Input::SharedInstance())->GetKey(KEYCODE_RIGHT))
		{
			mPlayer->mTransform.mPosition.x += mPlayerSpeed;
			UpdatePlayer();
		}
		else if ((&Input::SharedInstance())->GetKey(KEYCODE_UP))
		{
			mPlayer->mTransform.mPosition.y += mPlayerSpeed;
			UpdatePlayer();
		}
		else if ((&Input::SharedInstance())->GetKey(KEYCODE_DOWN))
		{
			mPlayer->mTransform.mPosition.y -= mPlayerSpeed;
			UpdatePlayer();
		}

	}

	void VRender() override
	{

		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

		ID3D11RenderTargetView* RTVs[2] = { *(mRenderer->GetRenderTargetView()), mShadowCastersRTV };
		
		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
		mDeviceContext->OMSetRenderTargets(2, RTVs, mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), white);
		mDeviceContext->ClearRenderTargetView(mShadowCastersRTV, transp);
		mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		RenderWalls();		
		RenderPlayer();
		RenderLightCircles();
		RenderShadowMask();
		
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

		ReleaseMacro(mCircleInputLayout);
		ReleaseMacro(mCircleVertexShader);
		ReleaseMacro(mCirclePixelShader);
		ReleaseMacro(mCircleInstanceBuffer);
		ReleaseMacro(mColorWeightInstanceBuffer);

		ReleaseMacro(mPlayerInstanceBuffer);

		ReleaseMacro(mShadowCastersRTV);
		ReleaseMacro(mShadowCastersMap);
		ReleaseMacro(mShadowCastersSRV);
		ReleaseMacro(mShadowsARTV);
		ReleaseMacro(mShadowsAMap);
		ReleaseMacro(mShadowsASRV);
		ReleaseMacro(mShadowsBRTV);
		ReleaseMacro(mShadowsBMap);
		ReleaseMacro(mShadowsBSRV);
		ReleaseMacro(mShadowsFinalRTV);
		ReleaseMacro(mShadowsFinalMap);
		ReleaseMacro(mShadowsFinalSRV);
		ReleaseMacro(mBillboardVertexShader);
		ReleaseMacro(mBillboardPixelShader);
		ReleaseMacro(mShadowCasterPixelShader); 
		ReleaseMacro(mShadowPixelShader);
		ReleaseMacro(mShadowVertexShader);
		ReleaseMacro(mBlendStateShadowMask);
		ReleaseMacro(mBlendStateShadowCalc);
		ReleaseMacro(mSamplerState);
		ReleaseMacro(mPointShaderBuffer);
	}

	void VOnResize() override 
	{
		ReleaseMacro(mShadowCastersRTV);
		ReleaseMacro(mShadowCastersMap);
		ReleaseMacro(mShadowCastersSRV);
		ReleaseMacro(mShadowsARTV);
		ReleaseMacro(mShadowsAMap);
		ReleaseMacro(mShadowsASRV);
		ReleaseMacro(mShadowsBRTV);
		ReleaseMacro(mShadowsBMap);
		ReleaseMacro(mShadowsBSRV);
		ReleaseMacro(mShadowsFinalRTV);
		ReleaseMacro(mShadowsFinalMap);
		ReleaseMacro(mShadowsFinalSRV);

		D3D11_TEXTURE2D_DESC shadowCastersTextureDesc;
		shadowCastersTextureDesc.Width = mRenderer->GetWindowWidth();
		shadowCastersTextureDesc.Height = mRenderer->GetWindowHeight();
		shadowCastersTextureDesc.ArraySize = 1;
		shadowCastersTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		shadowCastersTextureDesc.CPUAccessFlags = 0;
		shadowCastersTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		shadowCastersTextureDesc.MipLevels = 1;
		shadowCastersTextureDesc.MiscFlags = 0;
		shadowCastersTextureDesc.SampleDesc.Count = 1;
		shadowCastersTextureDesc.SampleDesc.Quality = 0;
		shadowCastersTextureDesc.Usage = D3D11_USAGE_DEFAULT;

		mDevice->CreateTexture2D(&shadowCastersTextureDesc, nullptr, &mShadowCastersMap);
		mDevice->CreateTexture2D(&shadowCastersTextureDesc, nullptr, &mShadowsAMap);
		mDevice->CreateTexture2D(&shadowCastersTextureDesc, nullptr, &mShadowsBMap);
		mDevice->CreateTexture2D(&shadowCastersTextureDesc, nullptr, &mShadowsFinalMap);

		D3D11_RENDER_TARGET_VIEW_DESC shadowCastersRTVDesc;
		shadowCastersRTVDesc.Format = shadowCastersTextureDesc.Format;
		shadowCastersRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		shadowCastersRTVDesc.Texture2D.MipSlice = 0;

		mRenderer->GetDevice()->CreateRenderTargetView(mShadowCastersMap, &shadowCastersRTVDesc, &mShadowCastersRTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mShadowsAMap, &shadowCastersRTVDesc, &mShadowsARTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mShadowsBMap, &shadowCastersRTVDesc, &mShadowsBRTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mShadowsFinalMap, &shadowCastersRTVDesc, &mShadowsFinalRTV);

		D3D11_SHADER_RESOURCE_VIEW_DESC shadowCastersSRVDesc;
		shadowCastersSRVDesc.Format = shadowCastersTextureDesc.Format;
		shadowCastersSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shadowCastersSRVDesc.Texture2D.MipLevels = 1;
		shadowCastersSRVDesc.Texture2D.MostDetailedMip = 0;

		mDevice->CreateShaderResourceView(mShadowCastersMap, &shadowCastersSRVDesc, &mShadowCastersSRV);
		mDevice->CreateShaderResourceView(mShadowsAMap, &shadowCastersSRVDesc, &mShadowsASRV);
		mDevice->CreateShaderResourceView(mShadowsBMap, &shadowCastersSRVDesc, &mShadowsBSRV);
		mDevice->CreateShaderResourceView(mShadowsFinalMap, &shadowCastersSRVDesc, &mShadowsFinalSRV);
	}
#pragma endregion

#pragma region Render
	void RenderWalls()
	{
		const UINT stride = sizeof(mat4f);
		const UINT offset = 0;
		mDeviceContext->IASetInputLayout(mQuadInputLayout);
		mDeviceContext->VSSetShader(mQuadVertexShader, NULL, 0);
		mDeviceContext->PSSetShader(mShadowCasterPixelShader, NULL, 0);

		mQuadShaderData.Color = gWallColor;
		mDeviceContext->UpdateSubresource(mQuadShaderBuffer, 0, NULL, &mQuadShaderData, 0, 0);
		mDeviceContext->VSSetConstantBuffers(0, 1, &mQuadShaderBuffer);

		mRenderer->VBindMesh(mWallMesh);
		mDeviceContext->IASetVertexBuffers(1, 1, &mWallInstanceBuffer, &stride, &offset);

		mDeviceContext->DrawIndexedInstanced(mWallMesh->GetIndexCount(), mWallCount, 0, 0, 0);
	}

	void RenderLightCircles()
	{
		const UINT stride = sizeof(mat4f);
		const UINT offset = 0;

		const UINT Colorstride = sizeof(float);

		mDeviceContext->IASetInputLayout(mCircleInputLayout);
		mDeviceContext->VSSetShader(mCircleVertexShader, NULL, 0);
		mDeviceContext->PSSetShader(mQuadPixelShader, NULL, 0);

		mQuadShaderData.Color = gCircleColor;
		mDeviceContext->UpdateSubresource(mQuadShaderBuffer, 0, NULL, &mQuadShaderData, 0, 0);
		mDeviceContext->VSSetConstantBuffers(0, 1, &mQuadShaderBuffer);

		mRenderer->VBindMesh(mCircleMesh);
		mDeviceContext->IASetVertexBuffers(1, 1, &mCircleInstanceBuffer, &stride, &offset);
		mDeviceContext->IASetVertexBuffers(2, 1, &mColorWeightInstanceBuffer, &Colorstride, &offset);

		mDeviceContext->DrawIndexedInstanced(mCircleMesh->GetIndexCount(), mCircleCount, 0, 0, 0);
		
		//Light shadows
		mDeviceContext->ClearRenderTargetView(mShadowsFinalRTV, black);
		for (int i = 0; i < mCircleCount; i++) {
			if (mCircleColorWeights[i] == 0) continue;
			
			mat4f clip = (mCircleTransforms[i].transpose() * mQuadShaderData.View.transpose()) * mQuadShaderData.Projection.transpose();
			vec4f zero = { 0.0f, 0.0f, 14.0f, 1.0f };
			mPointShaderData.Point = zero * clip;

			mDeviceContext->ClearRenderTargetView(mShadowsARTV, white);
			mDeviceContext->ClearRenderTargetView(mShadowsBRTV, white);

			mDeviceContext->IASetInputLayout(mQuadInputLayout);
			mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
			mDeviceContext->VSSetShader(mBillboardVertexShader, nullptr, 0);
			mDeviceContext->PSSetShader(mShadowPixelShader, nullptr, 0);
			mDeviceContext->UpdateSubresource(mPointShaderBuffer, 0, NULL, &mPointShaderData, 0, 0);
			mDeviceContext->PSSetConstantBuffers(0, 1, &mPointShaderBuffer);

			//mDeviceContext->ClearRenderTargetView(mShadowCastersBRTV, transp);

			mDeviceContext->OMSetRenderTargets(1, &mShadowsBRTV, nullptr);
			mDeviceContext->PSSetShaderResources(0, 1, &mShadowCastersSRV);

			int p;
			for (p = 0; p <= 1; p++) {
				if (p != 0) mDeviceContext->OMSetRenderTargets(1, p % 2 == 0 ? &mShadowsBRTV : &mShadowsARTV, nullptr);
				if (p != 0) mDeviceContext->PSSetShaderResources(0, 1, p % 2 == 0 ? &mShadowsASRV : &mShadowsBSRV);
				mDeviceContext->Draw(3, 0);
				mDeviceContext->PSSetShaderResources(0, 1, nullSRV);
			}

			mDeviceContext->OMSetBlendState(mBlendStateShadowCalc, nullptr, ~0);
			mDeviceContext->VSSetShader(mBillboardVertexShader, nullptr, 0);
			mDeviceContext->PSSetShader(mBillboardPixelShader, nullptr, 0);
			mDeviceContext->OMSetRenderTargets(1, &mShadowsFinalRTV, nullptr);
			mDeviceContext->PSSetShaderResources(0, 1, p % 2 == 0 ? &mShadowsASRV : &mShadowsBSRV);
			mDeviceContext->Draw(3, 0);
			mDeviceContext->PSSetShaderResources(0, 1, nullSRV);
			mDeviceContext->OMSetBlendState(nullptr, nullptr, ~0);
		}
	}

	void RenderShadowMask() {
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
		mDeviceContext->VSSetShader(mBillboardVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mBillboardPixelShader, nullptr, 0);

		mDeviceContext->OMSetBlendState(mBlendStateShadowMask, nullptr, ~0);
		mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr);
		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
		mDeviceContext->PSSetShaderResources(0, 1, &mShadowsFinalSRV);
		mDeviceContext->Draw(3, 0);
		mDeviceContext->PSSetShaderResources(0, 1, nullSRV);
		mDeviceContext->OMSetBlendState(nullptr, nullptr, ~0);
	}

	void RenderPlayer()
	{
		const UINT stride = sizeof(mat4f);
		const UINT offset = 0;
		mDeviceContext->IASetInputLayout(mQuadInputLayout);
		mDeviceContext->VSSetShader(mQuadVertexShader, NULL, 0);
		mDeviceContext->PSSetShader(mShadowCasterPixelShader, NULL, 0);

		mQuadShaderData.Color = gPlayerColor;
		mDeviceContext->UpdateSubresource(mQuadShaderBuffer, 0, NULL, &mQuadShaderData, 0, 0);
		mDeviceContext->VSSetConstantBuffers(0, 1, &mQuadShaderBuffer);

		mRenderer->VBindMesh(mWallMesh);
		mDeviceContext->IASetVertexBuffers(1, 1, &mPlayerInstanceBuffer, &stride, &offset);

		mDeviceContext->DrawIndexedInstanced(mWallMesh->GetIndexCount(), 1, 0, 0, 0);
	}

	void UpdatePlayer() {
		mPlayerTransform = (mat4f::rotateY(PI) *mat4f::translate(mPlayer->mTransform.mPosition)).transpose();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mDeviceContext->Map(mPlayerInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		// Copy the instances array into the instance buffer.
		memcpy(mappedResource.pData, &mPlayerTransform, sizeof(mat4f));
		// Unlock the instance buffer.
		mDeviceContext->Unmap(mPlayerInstanceBuffer, 0);
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
		LoadTransforms(&mCircleTransforms, &levelReader.mLights[0], NULL, NULL, levelReader.mLights.size(), 1);
		mCircleCount = levelReader.mLights.size();
		mLightPos = levelReader.mLights;
		
		mCircleColorWeights = (float*)mSceneAllocator.Allocate(sizeof(float)*mCircleCount,alignof(float),0);
		//for now, only the 5 first lights are "lit"
		for (int i = 0;  i < mCircleCount; i++)
		{
			if (i < 5)
				mCircleColorWeights[i] = 1.0f;
			else
				mCircleColorWeights[i] = 0.0f;
		}

		// Player
		mPlayer = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject), alignof(SceneObject), 0));
		mPlayer->mTransform.mPosition = levelReader.mPlayerPos;
		mPlayer->mTransform.mPosition.x += 1;
		//Not using Transform.GetWorldMatrix because Scale, rotation and bleh
		mPlayerTransform = (mat4f::rotateY(PI) *mat4f::translate(mPlayer->mTransform.mPosition)).transpose();


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
				(*transforms)[i] = (mat4f::rotateY(PI) *mat4f::translate(positions[i])).transpose();
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

	void InitializeWallShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
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

		D3DReadFileToBlob(L"BillboardVertexShader.cso", &psBlob);
		mDevice->CreateVertexShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mBillboardVertexShader);

		D3DReadFileToBlob(L"BillboardPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mBillboardPixelShader);

		D3D11_BUFFER_DESC pointDesc;
		pointDesc.ByteWidth = sizeof(PointShaderData);
		pointDesc.Usage = D3D11_USAGE_DEFAULT;
		pointDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		pointDesc.CPUAccessFlags = 0;
		pointDesc.MiscFlags = 0;
		pointDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&pointDesc, NULL, &mPointShaderBuffer);

		D3DReadFileToBlob(L"ShadowPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mShadowPixelShader);

		D3DReadFileToBlob(L"ShadowVertexShader.cso", &psBlob);
		mDevice->CreateVertexShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mShadowVertexShader);

		D3DReadFileToBlob(L"ShadowCasterPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mShadowCasterPixelShader);

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);

		D3D11_BLEND_DESC blendDesc;
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0] = {
			true,
			D3D11_BLEND_SRC_ALPHA,
			D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_OP_ADD,
			D3D11_BLEND_ONE,
			D3D11_BLEND_ZERO,
			D3D11_BLEND_OP_ADD,
			D3D11_COLOR_WRITE_ENABLE_ALL
		};

		mDevice->CreateBlendState(&blendDesc, &mBlendStateShadowMask);

		D3D11_BLEND_DESC blendDesc2;
		blendDesc2.AlphaToCoverageEnable = false;
		blendDesc2.IndependentBlendEnable = false;
		blendDesc2.RenderTarget[0] = {
			true,
			D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND_DEST_COLOR,
			D3D11_BLEND_OP_ADD,
			D3D11_BLEND_SRC_ALPHA,
			D3D11_BLEND_DEST_ALPHA,
			D3D11_BLEND_OP_MIN,
			D3D11_COLOR_WRITE_ENABLE_ALL
		};

		mDevice->CreateBlendState(&blendDesc2, &mBlendStateShadowCalc);

	}

	void InitializeLightShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"CircleVertexShader.cso", &vsBlob);

		// Create the shader on the device
		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&mCircleVertexShader);

		// Before cleaning up the data, create the input layout
		if (inputDescription) {
			mDevice->CreateInputLayout(
				inputDescription,					// Reference to Description
				6,									// Number of elments inside of Description
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mCircleInputLayout);
		}

		// Clean up
		vsBlob->Release();

		// Instance buffer
		D3D11_BUFFER_DESC circleInstanceBufferDesc;
		circleInstanceBufferDesc.ByteWidth = sizeof(mat4f) * mCircleCount;
		circleInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		circleInstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		circleInstanceBufferDesc.CPUAccessFlags = 0;
		circleInstanceBufferDesc.MiscFlags = 0;
		circleInstanceBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA instanceData;
		instanceData.pSysMem = mCircleTransforms;

		mDevice->CreateBuffer(&circleInstanceBufferDesc, &instanceData, &mCircleInstanceBuffer);
		
		// Coloer Weight buffer
		D3D11_BUFFER_DESC colorWeightsInstanceBufferDesc;
		colorWeightsInstanceBufferDesc.ByteWidth = sizeof(float) * mCircleCount;
		colorWeightsInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		colorWeightsInstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		colorWeightsInstanceBufferDesc.CPUAccessFlags = 0;
		colorWeightsInstanceBufferDesc.MiscFlags = 0;
		colorWeightsInstanceBufferDesc.StructureByteStride = 0;

		//D3D11_SUBRESOURCE_DATA instanceData;
		instanceData.pSysMem = mCircleColorWeights;

		mDevice->CreateBuffer(&colorWeightsInstanceBufferDesc, &instanceData, &mColorWeightInstanceBuffer);
	}

	void InitializePlayerShaders() {
		//Mostly, re-using Walls shaders for now (or forever)
		// Instance buffer
		D3D11_BUFFER_DESC playerInstanceBufferDesc;
		playerInstanceBufferDesc.ByteWidth = sizeof(mat4f);
		playerInstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		playerInstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		playerInstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		playerInstanceBufferDesc.MiscFlags = 0;
		playerInstanceBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA instanceData;
		instanceData.pSysMem = &(mPlayerTransform);

		mDevice->CreateBuffer(&playerInstanceBufferDesc, &instanceData, &mPlayerInstanceBuffer);
	}

#pragma endregion 
};

DECLARE_MAIN(Proto_03_Remix);