#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\LinearAllocator.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include "PathFinder/Fringe.h"
#include "SceneObject.h"
#include "LevelReader.h"
#include "Rig3D/GraphicsMath/cgm.h"
#include "Vertex.h"
#include "Grid.h"
#include "Rig3D/Intersection.h"
#include "Rig3D/Parametric.h"
#include "TargetFollower.h"
#include "Colors.h"

#define PI					3.1415926535f
#define UNITY_QUAD_RADIUS	0.85f

#if defined _DEBUG

std::function<void(const vec3f&, const vec3f&, const vec4f&)> __gTraceLine;
void TraceLine(const vec3f& from, const vec3f& to, const vec4f& color)
{
	__gTraceLine(from, to, color);
}

static const int gLineTraceMaxCount = 12000;
#else
static const int gLineTraceMaxCount = 0;
#endif

static const int gLineTraceVertexCount = 2 * gLineTraceMaxCount;

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

static const vec4f gWallColor = { 1.0f, 0.0f, 1.0f, 1.0f };
static const vec4f gCircleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
static const vec4f gPlayerColor = { 0.0f, 0.0f, 1.0f, 1.0f };
static const int gCircleVertexCount = 13;
static const int gCircleIndexCount = 36;	// Indices = (vertices - 1) * 3

// __gLineTraceVertexCount is set to 0 if _DEBUG is not defined
static const int gSceneMemorySize = 20480 + (gLineTraceVertexCount * 8 * 4);
static const int gMeshMemorySize = 1024;

char gSceneMemory[gSceneMemorySize];
char gStaticMeshMemory[gMeshMemorySize];
char gDynamicMeshMemory[gMeshMemorySize];

class Proto_03_Remix : public IScene, public virtual IRendererDelegate
{


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

	struct LineTraceShaderData
	{
		mat4f View;
		mat4f Projection;
	};

	struct LineTraceVertex
	{
		vec4f Color;
		vec3f Position;
	};

	QuadShaderData					mQuadShaderData;
	LineTraceShaderData				mLineTraceShaderData;
	PointShaderData					mPointShaderData;

	mat4f							mViewMatrix;
	mat4f							mProjectionMatrix;

	Transform						mPlayerTransform;
	Transform						mGoalTransform;

	LinearAllocator					mSceneAllocator;
	LinearAllocator					mStaticMeshAllocator;
	LinearAllocator					mDynamicMeshAllocator; // TO DO: This will likely need to be a Pool Allocator ->Gabe

	MeshLibrary<LinearAllocator>	mStaticMeshLibrary;
	MeshLibrary<LinearAllocator>	mDynamicMeshLibrary;

	SceneObject*					mWalls;
	SceneObject*					mBlocks;
	SceneObject*					mWaypoints;
	SceneObject*					mLights;

	SceneObject						mPlayer;
	SceneObject						mGoal;

	BoxCollider						mPlayerCollider;

	vector<RobotInfo>				mRobots;

	AABB<vec2f>*					mAABBs;
	Sphere<vec3f>*					mLightColliders;

	LineTraceVertex*				mLineTraceVertices;
	int								mLineTraceDrawCount;
	bool							mLineTraceOverflow;

	mat4f*							mWallTransforms;
	mat4f*							mBlockTransforms;
	mat4f*							mCircleTransforms;
	mat4f*							mRobotTransforms;
	float*							mCircleColorWeights;

	std::vector<vec3f>				mLightPos;

	BoxCollider*					mWallColliders;

	int								mWallCount;
	int								mBlockCount;
	int								mCircleCount;
	int								mRobotCount;
	int								mAABBCount;

	IMesh*							mWallMesh;
	IMesh*							mRobotMesh;
	IMesh*							mCircleMesh;
	IMesh*							mLightMesh;
	IMesh*							mPlayerMesh;
	IMesh*							mLineTraceMesh;
	IMesh*							mGridMesh;

	DX3D11Renderer*					mRenderer;

	ID3D11Device*					mDevice;
	ID3D11DeviceContext*			mDeviceContext;

	ID3D11InputLayout*				mQuadInputLayout;
	ID3D11VertexShader*				mQuadVertexShader;
	ID3D11PixelShader*				mQuadPixelShader;
	ID3D11Buffer*					mQuadShaderBuffer;
	ID3D11Buffer*					mWallInstanceBuffer;

	ID3D11InputLayout*				mLineTraceInputLayout;
	ID3D11VertexShader*				mLineTraceVertexShader;
	ID3D11PixelShader*				mLineTracePixelShader;
	ID3D11Buffer*					mLineTraceShaderBuffer;

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
	ID3D11PixelShader*				mShadowPixelShader;
	ID3D11PixelShader*				mGridPixelShader;
	ID3D11RenderTargetView*			mGridRTV;
	ID3D11Texture2D*				mGridMap;
	ID3D11SamplerState*				mSamplerState;
	ID3D11BlendState*				mBlendStateShadowMask;
	ID3D11BlendState*				mBlendStateShadowCalc;
	ID3D11Buffer*					mPointShaderBuffer;


	ID3D11ComputeShader*		mShadowGridComputeShader;
	ID3D11ShaderResourceView*	mSrcDataGPUBufferView0;
	ID3D11ShaderResourceView*	mSrcDataGPUBufferView1;

	ID3D11Buffer*				mDestDataGPUBuffer;
	ID3D11Buffer*				mDestDataGPUBufferCPURead;
	ID3D11UnorderedAccessView*	mDestDataGPUBufferView;


	float white[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float transp[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float black[4] = { 0.0f, 0.0f, 0.0f, 0.5f };

	// singletons
	Input& mInput = Input::SharedInstance();
	Grid& mGrid = Grid::SharedInstance();

	ID3D11ShaderResourceView* nullSRV[2] = { 0, 0 };

#pragma region IScene Override

	Proto_03_Remix() :
		mSceneAllocator(static_cast<void*>(gSceneMemory), static_cast<void*>(gSceneMemory + gSceneMemorySize)),
		mStaticMeshAllocator(static_cast<void*>(gStaticMeshMemory), static_cast<void*>(gStaticMeshMemory + gMeshMemorySize)),
		mDynamicMeshAllocator(static_cast<void*>(gDynamicMeshMemory), static_cast<void*>(gDynamicMeshMemory + gMeshMemorySize))
	{
		mOptions.mWindowCaption = "Shutter - Remix";
		mOptions.mWindowWidth = 1600;
		mOptions.mWindowHeight = 1000;
		mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen = false;
		mStaticMeshLibrary.SetAllocator(&mStaticMeshAllocator);
		mDynamicMeshLibrary.SetAllocator(&mDynamicMeshAllocator);

#if defined _DEBUG
		__gTraceLine = [this](const vec3f& from, const vec3f& to, const vec4f& color) { TraceLine(from, to, color); };
#endif
	}

	~Proto_03_Remix() {}

	void VInitialize() override
	{
		mRenderer = &DX3D11Renderer::SharedInstance();
		mDeviceContext = mRenderer->GetDeviceContext();
		mDevice = mRenderer->GetDevice();

		mRenderer->SetDelegate(this);
		VOnResize();
		
		InitializeLevel();
		InitializeGrid();
		InitializeGeometry();
		InitializeLineTraceShaders();
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
		HandleInput(Input::SharedInstance());

		bool moved = false;

		auto target = mRobots[4];

		auto start = mPlayer.mTransform->GetPosition(); //Vector3(10, 20, 0);
		auto end = target.Transform.GetPosition();
		//grid.GetPath(start, end);
		
		auto from = mGrid.GetNodeAt(start);
		auto to = mGrid.GetNodeAt(end);
			
		auto result = mGrid.pathFinder.FindPath(&to, &from);
		static TargetFollower follower(*mPlayer.mTransform, mAABBs, mAABBCount);
		follower.MoveTowards(target.Transform);
		
		if (result.path.size() > 0)
		{
			auto it = result.path.begin();
			auto p1 = **it;
			for (++it; it != result.path.end(); ++it)
			{
				auto p2 = **it;
				TraceLine(p1.position, p2.position, Colors::blue);
				p1 = p2;
			}
		}

		
		UpdateGrid();
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
		RenderRobots();
		RenderGrid();

		RenderShadowMask();

		RenderGridToBuffer();

		// changes the primitive type to lines
		RenderLineTrace();

		mRenderer->VSwapBuffers();
	}

	void VShutdown() override {
		mCircleMesh->~IMesh();
		mWallMesh->~IMesh();
		mLineTraceMesh->~IMesh();
		mGridMesh->~IMesh();

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

		ReleaseMacro(mLineTraceInputLayout);
		ReleaseMacro(mLineTraceVertexShader);
		ReleaseMacro(mLineTracePixelShader);
		ReleaseMacro(mLineTraceShaderBuffer);

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
		ReleaseMacro(mGridPixelShader);
		ReleaseMacro(mGridMap);
		ReleaseMacro(mGridRTV);
		ReleaseMacro(mBlendStateShadowMask);
		ReleaseMacro(mBlendStateShadowCalc);
		ReleaseMacro(mSamplerState);
		ReleaseMacro(mPointShaderBuffer);

		ReleaseMacro(mShadowGridComputeShader);
		ReleaseMacro(mSrcDataGPUBufferView0);
		ReleaseMacro(mSrcDataGPUBufferView1);
		ReleaseMacro(mDestDataGPUBuffer);
		ReleaseMacro(mDestDataGPUBufferCPURead);
		ReleaseMacro(mDestDataGPUBufferView);
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
		ReleaseMacro(mDestDataGPUBuffer);
		ReleaseMacro(mDestDataGPUBufferCPURead);
		ReleaseMacro(mDestDataGPUBufferView);
		ReleaseMacro(mSrcDataGPUBufferView0);
		ReleaseMacro(mSrcDataGPUBufferView1);
		ReleaseMacro(mGridMap);
		ReleaseMacro(mGridRTV);


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
		mDevice->CreateTexture2D(&shadowCastersTextureDesc, nullptr, &mGridMap);


		D3D11_RENDER_TARGET_VIEW_DESC shadowCastersRTVDesc;
		shadowCastersRTVDesc.Format = shadowCastersTextureDesc.Format;
		shadowCastersRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		shadowCastersRTVDesc.Texture2D.MipSlice = 0;

		mRenderer->GetDevice()->CreateRenderTargetView(mShadowCastersMap, &shadowCastersRTVDesc, &mShadowCastersRTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mShadowsAMap, &shadowCastersRTVDesc, &mShadowsARTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mShadowsBMap, &shadowCastersRTVDesc, &mShadowsBRTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mShadowsFinalMap, &shadowCastersRTVDesc, &mShadowsFinalRTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mGridMap, &shadowCastersRTVDesc, &mGridRTV);

		D3D11_SHADER_RESOURCE_VIEW_DESC shadowCastersSRVDesc;
		shadowCastersSRVDesc.Format = shadowCastersTextureDesc.Format;
		shadowCastersSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shadowCastersSRVDesc.Texture2D.MipLevels = 1;
		shadowCastersSRVDesc.Texture2D.MostDetailedMip = 0;

		mDevice->CreateShaderResourceView(mShadowCastersMap, &shadowCastersSRVDesc, &mShadowCastersSRV);
		mDevice->CreateShaderResourceView(mShadowsAMap, &shadowCastersSRVDesc, &mShadowsASRV);
		mDevice->CreateShaderResourceView(mShadowsBMap, &shadowCastersSRVDesc, &mShadowsBSRV);
		mDevice->CreateShaderResourceView(mShadowsFinalMap, &shadowCastersSRVDesc, &mShadowsFinalSRV);

		{
			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			descGPUBuffer.ByteWidth = numSpheresX*numSpheresY * sizeof(int);
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(int);

			mDevice->CreateBuffer(&descGPUBuffer, NULL, &mDestDataGPUBuffer);

			descGPUBuffer.Usage = D3D11_USAGE_STAGING;
			descGPUBuffer.BindFlags = 0;
			descGPUBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			mDevice->CreateBuffer(&descGPUBuffer, NULL, &mDestDataGPUBufferCPURead);

			D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			descView.Buffer.NumElements = numSpheresX*numSpheresY;

			mDevice->CreateUnorderedAccessView(mDestDataGPUBuffer, &descView, &mDestDataGPUBufferView);
		}

		{
			D3D11_SHADER_RESOURCE_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.Texture2D.MipLevels = 1;
			descView.Texture2D.MostDetailedMip = 0;

			mDevice->CreateShaderResourceView(mShadowsFinalMap, &descView, &mSrcDataGPUBufferView0);
			mDevice->CreateShaderResourceView(mGridMap, &descView, &mSrcDataGPUBufferView1);
		}
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
			mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr);
		}

		//Compute 
		mDeviceContext->CSSetShader(mShadowGridComputeShader, NULL, 0);
		mDeviceContext->CSSetShaderResources(0, 1, &mSrcDataGPUBufferView0);
		mDeviceContext->CSSetShaderResources(1, 1, &mSrcDataGPUBufferView1);
		mDeviceContext->CSSetUnorderedAccessViews(0, 1, &mDestDataGPUBufferView, NULL);
		mDeviceContext->Dispatch(mRenderer->GetWindowWidth()/32, mRenderer->GetWindowHeight()/32, 1);
		mDeviceContext->CSSetShader(NULL, NULL, 0);
		mDeviceContext->CSSetShaderResources(0, 2, nullSRV);
		mDeviceContext->CopyResource(mDestDataGPUBufferCPURead, mDestDataGPUBuffer);

		//Map and update
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mDeviceContext->Map(mDestDataGPUBufferCPURead, 0, D3D11_MAP_READ, 0, &mappedResource);
		int* ints = reinterpret_cast<int*>(mappedResource.pData);
		auto g = mGrid.pathFinder.graph.grid;
		for (int i = 0; i < numSpheresX; i++)
		{
			for (int j = 0; j < numSpheresY; j++)
			{
				g[i][j].hasLight = ints[i+j*numSpheresX] == 1;
			}
		}
		mDeviceContext->Unmap(mDestDataGPUBufferCPURead, 0);
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

	void RenderRobots()
	{
		for (auto robot : mRobots)
		{
			const auto pone = vec3f(.5f, .5f, 0);
			const auto none = vec3f(-.5f, .5f, 0);
			auto pos = robot.Transform.GetPosition();

			TRACE_BOX(pos, Colors::red);

			for (int i = 1, len = robot.Waypoints.size(); i < len; i++)
			{
				TraceLine(robot.Waypoints[i - 1], robot.Waypoints[i], Colors::green);
			}
		}
	}

	void RenderGrid()
	{
		for (int i = 0; i < numSpheresX; i++)
		{
			for (int j = 0; j < numSpheresY; j++)
			{
				const auto pone = vec3f(nodeRadius, nodeRadius, 0);
				const auto none = vec3f(-nodeRadius, nodeRadius, 0);
				auto pos = mGrid.pathFinder.graph.grid[i][j].position;
				auto hasLight = mGrid.pathFinder.graph.grid[i][j].hasLight;

				if (!hasLight) {
					TraceLine(pos + pone, pos - pone, Colors::magenta);
					TraceLine(pos + none, pos - none, Colors::magenta);
				}
			}
		}
	}

	void RenderLineTrace()
	{
		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_LINE);

		mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), nullptr);

		mDeviceContext->IASetInputLayout(mLineTraceInputLayout);
		mDeviceContext->VSSetShader(mLineTraceVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mLineTracePixelShader, nullptr, 0);

		mDeviceContext->UpdateSubresource(static_cast<DX11Mesh*>(mLineTraceMesh)->mVertexBuffer, 0, nullptr, mLineTraceVertices, 0, 0);
		mDeviceContext->UpdateSubresource(mLineTraceShaderBuffer, 0, nullptr, &mLineTraceShaderData, 0, 0);
		mDeviceContext->VSSetConstantBuffers(0, 1, &mLineTraceShaderBuffer);

		mRenderer->VBindMesh(mLineTraceMesh);
		mRenderer->VDrawIndexed(0, mLineTraceDrawCount);

		// reset line trace count
		mLineTraceDrawCount = 0;
		mLineTraceOverflow = false;
	}


	void RenderGridToBuffer()
	{
		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_POINT);

		mDeviceContext->OMSetRenderTargets(1, &mGridRTV, nullptr);

		mDeviceContext->IASetInputLayout(mLineTraceInputLayout);
		mDeviceContext->VSSetShader(mLineTraceVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mGridPixelShader, nullptr, 0);

		//mDeviceContext->UpdateSubresource(static_cast<DX11Mesh*>(mGridMesh)->mVertexBuffer, 0, nullptr, &mGridVertices, 0, 0);
		mDeviceContext->UpdateSubresource(mLineTraceShaderBuffer, 0, nullptr, &mLineTraceShaderData, 0, 0);
		mDeviceContext->VSSetConstantBuffers(0, 1, &mLineTraceShaderBuffer);

		mRenderer->VBindMesh(mGridMesh);
		mRenderer->VDrawIndexed(0, numSpheresX * numSpheresY);
	}

	void UpdatePlayer() {
		mat4f playerWorldMatrix = mPlayer.mTransform->GetWorldMatrix().transpose();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mDeviceContext->Map(mPlayerInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		// Copy the instances array into the instance buffer.
		memcpy(mappedResource.pData, &playerWorldMatrix, sizeof(mat4f));
		// Unlock the instance buffer.
		mDeviceContext->Unmap(mPlayerInstanceBuffer, 0);
	}

	void UpdateGrid()
	{
		for (size_t x = 0; x < numSpheresX; x++)
		{
			for (size_t y = 0; y < numSpheresY; y++)
			{
				auto node = &mGrid.graph.grid[x][y];

				node->weight = node->hasLight ? 0 : FLT_MAX;

				if (node->weight > 1)
				{
					TRACE_BOX(node->position, Colors::magenta);
				}
			}
		}

	}

	void UpdateRobotTransforms()
	{
		mRobotCount = mRobots.size();
		mRobotTransforms = reinterpret_cast<mat4f*>(mSceneAllocator.Allocate(sizeof(mat4f) * mRobotCount, alignof(mat4f), 0));

		for (int i = 0; i < mRobotCount; i++)
		{
			auto robot = &mRobots[i];

			mRobotTransforms[i] = robot->Transform.GetWorldMatrix();
		}
	}

#pragma endregion 

#pragma region Initialization

	void InitializeCamera()
	{
		float aspectRatio = static_cast<float>(mOptions.mWindowWidth) / mOptions.mWindowHeight;
		float halfHeight = 33.5f;
		float halfWidth = 33.5f * aspectRatio;
		mProjectionMatrix = mat4f::normalizedOrthographicLH(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.3f, 1000.0f).transpose();
		mViewMatrix = mat4f::lookAtLH(vec3f(0.0f, 0.0f, 1.0f), vec3f(0.0f, 0.0f, -30.0f), vec3f(0.0f, 1.0f, 0.0f)).transpose();

		mQuadShaderData.View = mViewMatrix;
		mQuadShaderData.Projection = mProjectionMatrix;

		mLineTraceShaderData.View = mViewMatrix;
		mLineTraceShaderData.Projection = mProjectionMatrix;
	}

	void InitializeLevel()
	{
		LevelReader levelReader("UnitySceneExport.json");
		levelReader.ReadLevel();

		// Walls
		mWallCount = levelReader.mWalls.Position.size();
		LoadStaticSceneObjects(&mWalls, &mWallTransforms, &mWallColliders, &levelReader.mWalls.Position[0], &levelReader.mWalls.Rotation[0], &levelReader.mWalls.Scale[0], mWallCount);
		
		// Blocks
		mBlockCount = levelReader.mBlocks.Position.size();
		LoadTransforms(&mBlockTransforms, &levelReader.mBlocks.Position[0], &levelReader.mBlocks.Rotation[0], &levelReader.mBlocks.Scale[0], mBlockCount, 0);


		// Note: We are allocating aabbs twice!!! Maybe we should allocate our data structures before filling them out with level data?
		mAABBCount = mWallCount + mBlockCount;
		mAABBs = reinterpret_cast<AABB<vec2f>*>(mSceneAllocator.Allocate(sizeof(AABB<vec2f>) * (mAABBCount), alignof(AABB<vec2f>), 0));
		SetAABBs(levelReader.mWalls, mAABBs, 0);
		SetAABBs(levelReader.mBlocks, mAABBs, mWallCount);

		// Lights
		mCircleCount = levelReader.mLights.size();
		LoadLights(&mLights, &mCircleTransforms, &mLightColliders, &levelReader.mLights[0], mCircleCount);
	//	LoadTransforms(&mCircleTransforms, &levelReader.mLights[0], nullptr, nullptr, mCircleCount, 1);
		mLightPos = levelReader.mLights;

		mCircleColorWeights = (float*)mSceneAllocator.Allocate(sizeof(float)*mCircleCount, alignof(float), 0);
		//for now, only the 5 first lights are "lit"
		for (int i = 0; i < mCircleCount; i++)
		{
			if (i < 5)
				mCircleColorWeights[i] = 1.0f;
			else
				mCircleColorWeights[i] = 0.0f;
		}

		// Robots
		mRobots = levelReader.mRobots;
		UpdateRobotTransforms();

		// Player
		mPlayer.mTransform = &mPlayerTransform;
		mPlayer.mTransform->SetPosition(levelReader.mPlayerPos + vec3f(1, 0, 0));
		//mPlayer.mTransform->RotateYaw(PI);

		mPlayer.mBoxCollider = &mPlayerCollider;
		mPlayer.mBoxCollider->origin = mPlayerTransform.GetPosition();
		mPlayer.mBoxCollider->halfSize = vec3f(UNITY_QUAD_RADIUS) * mPlayerTransform.GetScale();

		// Goal
		mGoal.mTransform = &mGoalTransform;
		mGoal.mTransform->SetPosition(levelReader.mGoalPos);
		mGoal.mTransform->RotateYaw(PI);
	}

	void InitializeGrid()
	{

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
				(*transforms)[i] = (mat4f::scale(scales[i]) * mat4f::translate(positions[i])).transpose();
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

	void LoadStaticSceneObjects(SceneObject** sceneObjects, mat4f** transforms, BoxCollider** colliders, vec3f* positions, vec3f* rotations, vec3f* scales, int size)
	{
		// Allocate size SceneObjects
		*sceneObjects = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject) * size, alignof(SceneObject), 0));
		*transforms = reinterpret_cast<mat4f*>(mSceneAllocator.Allocate(sizeof(mat4f) * size, alignof(mat4f), 0));
		*colliders = reinterpret_cast<BoxCollider*>(mSceneAllocator.Allocate(sizeof(BoxCollider) * size, alignof(BoxCollider), 0));

		for (int i = 0; i < size; i++)
		{
			(*transforms)[i] = (mat4f::scale(scales[i]) * mat4f::translate(positions[i])).transpose();
			(*colliders)[i] = BoxCollider({ positions[i], vec3f(0.85f) * scales[i] });

			(*sceneObjects)[i].mWorldMatrix = transforms[i];
			(*sceneObjects)[i].mBoxCollider = colliders[i];
		}
	}

	void LoadLights(SceneObject** sceneObjects, mat4f** transforms, SphereCollider** colliders, vec3f* positions, int size)
	{
		// Allocate size SceneObjects
		*sceneObjects = reinterpret_cast<SceneObject*>(mSceneAllocator.Allocate(sizeof(SceneObject) * size, alignof(SceneObject), 0));
		*transforms = reinterpret_cast<mat4f*>(mSceneAllocator.Allocate(sizeof(mat4f) * size, alignof(mat4f), 0));
		*colliders = reinterpret_cast<SphereCollider*>(mSceneAllocator.Allocate(sizeof(SphereCollider) * size, alignof(SphereCollider), 0));

		for (int i = 0; i < size; i++)
		{
			(*transforms)[i] = mat4f::translate(positions[i]).transpose();
			(*colliders)[i] = SphereCollider({ positions[i], 1.0f });

			(*sceneObjects)[i].mWorldMatrix = transforms[i];
			(*sceneObjects)[i].mSphereCollider = colliders[i];
		}
	}

	static void SetAABBs(RectInfo rectInfo, AABB<Vector2>* aabb, int offset)
	{
		for (int i = 0; i < rectInfo.Position.size(); i++)
		{
			aabb[i + offset].origin = rectInfo.Position[i];
			aabb[i + offset].halfSize = rectInfo.Scale[i];
		}
	}

	void InitializeGeometry()
	{
		InitializeGridMesh();
		InitializeLineTraceMesh();
		InitializeQuadMesh();
		InitializeCircleMesh();
	}

	void InitializeLineTraceMesh()
	{
		mLineTraceVertices = reinterpret_cast<LineTraceVertex*>(mSceneAllocator.Allocate(sizeof(LineTraceVertex) * gLineTraceVertexCount, sizeof(LineTraceVertex), 0));

		uint16_t lineTraceIndices[gLineTraceVertexCount];
		for (auto i = 0; i < gLineTraceVertexCount; i++)
		{
			lineTraceIndices[i] = i;
		}

		mStaticMeshLibrary.NewMesh(&mLineTraceMesh, mRenderer);
		mRenderer->VSetMeshVertexBuffer(mLineTraceMesh, mLineTraceVertices, sizeof(LineTraceVertex) * gLineTraceVertexCount, sizeof(LineTraceVertex));
		mRenderer->VSetDynamicMeshIndexBuffer(mLineTraceMesh, lineTraceIndices, gLineTraceVertexCount);
	}

	void InitializeGridMesh()
	{
		auto grid = mGrid.pathFinder.graph.grid;
		const int gridCount = numSpheresX*numSpheresY;

		LineTraceVertex vertices[gridCount];

		for (auto i = 0; i < numSpheresX; i++)
		{
			for (auto j = 0; j < numSpheresY; j++)
			{
				vertices[i + j*numSpheresX].Position = grid[i][j].position;
				vertices[i + j*numSpheresX].Color = grid[i][j].coord;
				vertices[i + j*numSpheresX].Color.z = numSpheresX;
			}
		}

		uint16_t indices[gridCount];
		for (auto i = 0; i < gridCount; i++)
		{
			indices[i] = i;
		}

		mStaticMeshLibrary.NewMesh(&mGridMesh, mRenderer);
		mRenderer->VSetMeshVertexBuffer(mGridMesh, vertices, sizeof(LineTraceVertex) * gridCount, sizeof(LineTraceVertex));
		mRenderer->VSetDynamicMeshIndexBuffer(mGridMesh, indices, gridCount);
	}

	void InitializeQuadMesh()
	{
		vec3f quadVertices[4] =
		{
			{ -UNITY_QUAD_RADIUS, -UNITY_QUAD_RADIUS, 0.0f },
			{ +UNITY_QUAD_RADIUS, -UNITY_QUAD_RADIUS, 0.0f },
			{ +UNITY_QUAD_RADIUS, +UNITY_QUAD_RADIUS, 0.0f },
			{ -UNITY_QUAD_RADIUS, +UNITY_QUAD_RADIUS, 0.0f }
		};

		uint16_t quadIndices[6] = { 0, 2, 1, 3, 2, 0 };

		mStaticMeshLibrary.NewMesh(&mWallMesh, mRenderer);
		mRenderer->VSetStaticMeshVertexBuffer(mWallMesh, quadVertices, sizeof(vec3f) * 4, sizeof(vec3f));
		mRenderer->VSetStaticMeshIndexBuffer(mWallMesh, quadIndices, 6);
	}

	void InitializeCircleMesh()
	{
		vec3f circleVertices[gCircleVertexCount];
		float angularDisplacement = (2.0f * PI) / (gCircleVertexCount - 1);
		float radius = 1.0f;
		float angle = 0.0f;

		circleVertices[0] = { 0.0f, 0.0f, 0.0f };
		for (int i = 1; i < gCircleVertexCount; i++, angle += angularDisplacement)
		{
			circleVertices[i] = { radius * cosf(angle), radius * sinf(-angle), 0.0f };
		}

		uint16_t circleIndices[gCircleIndexCount] =
		{
			0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
			0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 9,
			0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 1
		};

		mStaticMeshLibrary.NewMesh(&mCircleMesh, mRenderer);
		mRenderer->VSetStaticMeshVertexBuffer(mCircleMesh, circleVertices, sizeof(vec3f) * gCircleVertexCount, sizeof(vec3f));
		mRenderer->VSetStaticMeshIndexBuffer(mCircleMesh, circleIndices, gCircleIndexCount);
	}

	void InitializeLineTraceShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"LineTraceVertexShader.cso", &vsBlob);

		// Create the shader on the device
		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&mLineTraceVertexShader);

		// Before cleaning up the data, create the input layout
		if (inputDescription) {
			mDevice->CreateInputLayout(
				inputDescription,					// Reference to Description
				2,									// Number of elments inside of Description
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mLineTraceInputLayout);
		}

		// Clean up
		vsBlob->Release();

		// Load Pixel Shader ---------------------------------------
		ID3DBlob* psBlob;
		D3DReadFileToBlob(L"LineTracePixelShader.cso", &psBlob);

		// Create the shader on the device
		mDevice->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			nullptr,
			&mLineTracePixelShader);

		// Clean up
		psBlob->Release();

		// Constant buffers ----------------------------------------
		D3D11_BUFFER_DESC lineTraceBufferDataDesc;
		lineTraceBufferDataDesc.ByteWidth = sizeof(LineTraceShaderData);
		lineTraceBufferDataDesc.Usage = D3D11_USAGE_DEFAULT;
		lineTraceBufferDataDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lineTraceBufferDataDesc.CPUAccessFlags = 0;
		lineTraceBufferDataDesc.MiscFlags = 0;
		lineTraceBufferDataDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&lineTraceBufferDataDesc, nullptr, &mLineTraceShaderBuffer);
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

		D3DReadFileToBlob(L"ShadowGridComputeShader.cso", &psBlob);
		mDevice->CreateComputeShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mShadowGridComputeShader);

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

		D3DReadFileToBlob(L"ShadowCasterPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mShadowCasterPixelShader);

		D3DReadFileToBlob(L"GridPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mGridPixelShader);


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

		//D3D11_SUBRESOURCE_DATA instanceData;
		//instanceData.pSysMem = &(mPlayerWorldMatrix);

		mDevice->CreateBuffer(&playerInstanceBufferDesc, nullptr, &mPlayerInstanceBuffer);
	}

#pragma endregion 

#pragma region Line Trace

	void TraceLine(const vec3f& from, const vec3f& to, const vec4f& color)
	{
		if (mLineTraceOverflow)
		{
			return;
		}

		if (mLineTraceDrawCount >= gLineTraceVertexCount - 1)
		{
			TRACE("[WARNING] Reached maximum number of traced lines. Increase __gLineTraceMaxCount to trace more lines.");
			mLineTraceOverflow = true;
			return;
		}

		auto index = mLineTraceDrawCount++;
		mLineTraceVertices[index].Position = from;
		mLineTraceVertices[index].Color = color;

		index = mLineTraceDrawCount++;
		mLineTraceVertices[index].Position = to;
		mLineTraceVertices[index].Color = color;
	}

#pragma endregion

	void HandleInput(Input& input)
	{
		// Player Mouse

		if (input.GetMouseButtonDown(MOUSEBUTTON_LEFT))
		{
			float x = (2.0f * input.mousePosition.x) / mRenderer->GetWindowWidth() - 1.0f;
			float y = 1.0f - (2.0f * input.mousePosition.y) / mRenderer->GetWindowHeight();

			mat4f toWorld = (mQuadShaderData.Projection * mQuadShaderData.View).inverse();
			vec3f worldPos = vec4f(x, y, 0.0f, 1.0f) * toWorld;
			vec3f worldDir = vec3f(0.0f, 0.0f, 1.0f);
			worldPos.z = -30.0f;

			Ray<vec3f> ray = { worldPos, worldDir };

			RayCastHit<vec3f> hit;
			if (RayCast(&hit, ray, mLightColliders, mCircleCount))
			{
				mCircleColorWeights[hit.index] = mCircleColorWeights[hit.index] > 0 ? 0 : 1;
			}
		}


		// Player Movement
		float mPlayerSpeed = 0.25f;

		auto pos = mPlayer.mTransform->GetPosition();
		if (input.GetKey(KEYCODE_LEFT))
		{
			pos.x -= mPlayerSpeed;
		}
		if (input.GetKey(KEYCODE_RIGHT))
		{
			pos.x += mPlayerSpeed;
		}
		if (input.GetKey(KEYCODE_UP))
		{
			pos.y += mPlayerSpeed;
		}
		if (input.GetKey(KEYCODE_DOWN))
		{
			pos.y -= mPlayerSpeed;
		}

		BoxCollider aabb = { pos, mPlayer.mBoxCollider->halfSize };

		for (int i = 0; i < mWallCount; i++)
		{
			if (IntersectAABBAABB(aabb, mWallColliders[i]))
			{
				return;
			}
		}

		mPlayer.mTransform->SetPosition(pos);
		mPlayer.mBoxCollider->origin = pos;
		UpdatePlayer();
	}
};

DECLARE_MAIN(Proto_03_Remix);
