#include "stdafx.h"
#include "capstone-game-cpp/ScareTacticsApplication.h"
#include "Level00.h"
#include "MainMenuScene.h"
#include <Rig3D/Graphics/DirectX11/DX11IMGUI.h>
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

static const vec4f gWallColor = { 1.0f, 0.0f, 1.0f, 1.0f };
static const vec4f gCircleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
static const vec4f gPlayerColor = { 0.0f, 0.0f, 1.0f, 1.0f };
static const int gCircleVertexCount = 13;
static const int gCircleIndexCount = 36;	// Indices = (vertices - 1) * 3

static const int gSceneMemorySize = 20480;
static const int gMeshMemorySize = 1024;

char gSceneMemory[gSceneMemorySize];
char gStaticMeshMemory[gMeshMemorySize];
char gDynamicMeshMemory[gMeshMemorySize];


#pragma region IScene Override

Level00::Level00() :
	mSceneAllocator(static_cast<void*>(gSceneMemory), static_cast<void*>(gSceneMemory + gSceneMemorySize)),
	mStaticMeshAllocator(static_cast<void*>(gStaticMeshMemory), static_cast<void*>(gStaticMeshMemory + gMeshMemorySize)),
	mDynamicMeshAllocator(static_cast<void*>(gDynamicMeshMemory), static_cast<void*>(gDynamicMeshMemory + gMeshMemorySize)),
	mWalls(nullptr),
	mBlocks(nullptr),
	mWaypoints(nullptr),
	mLights(nullptr),
	mPlayerNode(nullptr),
	mFollowers(nullptr),
	mAABBs(nullptr),
	mLightColliders(nullptr),
	mWallTransforms(nullptr),
	mBlockTransforms(nullptr),
	mCircleTransforms(nullptr),
	mRobotTransforms(nullptr),
	mCircleColorWeights(nullptr),
	mWallColliders(nullptr),
	mWallCount(0),
	mBlockCount(0),
	mCircleCount(0),
	mRobotCount(0),
	mAABBCount(0),
	mWallMesh(nullptr),
	mRobotMesh(nullptr),
	mCircleMesh(nullptr),
	mLightMesh(nullptr),
	mPlayerMesh(nullptr),
	mQuadInputLayout(nullptr),
	mQuadVertexShader(nullptr),
	mQuadPixelShader(nullptr),
	mQuadShaderBuffer(nullptr),
	mWallInstanceBuffer(nullptr),
	mCircleInputLayout(nullptr),
	mCircleVertexShader(nullptr),
	mCirclePixelShader(nullptr),
	mCircleInstanceBuffer(nullptr),
	mColorWeightInstanceBuffer(nullptr),
	mPlayerInstanceBuffer(nullptr),
	mShadowCastersRTV(nullptr),
	mShadowCastersMap(nullptr),
	mShadowCastersSRV(nullptr),
	mShadowsARTV(nullptr),
	mShadowsAMap(nullptr),
	mShadowsASRV(nullptr),
	mShadowsBRTV(nullptr),
	mShadowsBMap(nullptr),
	mShadowsBSRV(nullptr),
	mShadowsFinalRTV(nullptr),
	mShadowsFinalMap(nullptr),
	mShadowsFinalSRV(nullptr),
	mShadowCasterPixelShader(nullptr),
	mBillboardVertexShader(nullptr),
	mBillboardPixelShader(nullptr),
	mShadowPixelShader(nullptr),
	mSamplerState(nullptr),
	mBlendStateShadowMask(nullptr),
	mBlendStateShadowCalc(nullptr),
	mPointShaderBuffer(nullptr),
	mShadowGridComputeShader(nullptr),
	mSrcDataGPUBuffer(nullptr),
	mSrcDataGPUBufferView(nullptr),
	mDestDataGPUBuffer(nullptr),
	mDestDataGPUBufferCPURead(nullptr),
	mDestDataGPUBufferView(nullptr)
{
	mStaticMeshLibrary.SetAllocator(&mStaticMeshAllocator);
	mDynamicMeshLibrary.SetAllocator(&mDynamicMeshAllocator);

	mExplorer[0] = FactoryPool<Explorer>::Create();
	mExplorer[1] = FactoryPool<Explorer>::Create();
	mExplorer[2] = FactoryPool<Explorer>::Create();
	mExplorer[3] = FactoryPool<Explorer>::Create();
}

void Level00::VInitialize()
{
	mState = BASE_SCENE_STATE_INITIALIZING;

	VOnResize();

	InitializeLevel();
	InitializeGrid();
	InitializeRobots();
	InitializeGeometry();
	InitializeWallShaders();
	InitializeLightShaders();
	InitializePlayerShaders();
	InitializeCamera();

	mState = BASE_SCENE_STATE_RUNNING;

	if (mNetworkManager->mMode == NetworkManager::Mode::CLIENT) {
		Packet p(PacketTypes::INIT_CONNECTION);
		mNetworkManager->mClient.SendData(&p);
	}
}

void Level00::VUpdate(double milliseconds)
{
	HandleInput(*mInput);
	mNetworkManager->Update();

	UpdateGrid();
	UpdateRobots();
}

void Level00::VRender()
{
	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

	ID3D11RenderTargetView* RTVs[2] = { *(mRenderer->GetRenderTargetView()), mShadowCastersRTV };

	mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
	mDeviceContext->OMSetRenderTargets(2, RTVs, mRenderer->GetDepthStencilView());
	mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), black);
	mDeviceContext->ClearRenderTargetView(mShadowCastersRTV, transp);
	mDeviceContext->ClearDepthStencilView(mRenderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	mDeviceContext->OMSetBlendState(nullptr, nullptr, ~0);

	RenderWalls();
	RenderExplorer();
	RenderLightCircles();
	RenderRobots();
	//RenderGrid();

	RenderShadowMask();

	// changes the primitive type to lines
	RENDER_TRACE()

	//FPS
	DX11IMGUI::NewFrame();
	RenderFPSIndicator();
	ImGui::Render();

	mRenderer->VSwapBuffers();
}

void Level00::VShutdown()
{
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
	ReleaseMacro(mBlendStateShadowMask);
	ReleaseMacro(mBlendStateShadowCalc);
	ReleaseMacro(mSamplerState);
	ReleaseMacro(mPointShaderBuffer);

	ReleaseMacro(mShadowGridComputeShader);
	ReleaseMacro(mSrcDataGPUBuffer);
	ReleaseMacro(mSrcDataGPUBufferView);
	ReleaseMacro(mDestDataGPUBuffer);
	ReleaseMacro(mDestDataGPUBufferCPURead);
	ReleaseMacro(mDestDataGPUBufferView);
}

void Level00::VOnResize()
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
	ReleaseMacro(mSrcDataGPUBufferView);


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

	{
		D3D11_BUFFER_DESC descGPUBuffer;
		ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
		descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descGPUBuffer.ByteWidth = numSpheresX*numSpheresY * sizeof(GridNode);
		descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descGPUBuffer.StructureByteStride = sizeof(GridNode);

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
		D3D11_BUFFER_DESC descGPUBuffer;
		ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
		descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descGPUBuffer.ByteWidth = numSpheresX*numSpheresY * sizeof(GridNode);
		descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descGPUBuffer.StructureByteStride = sizeof(GridNode);

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = mGridNodeData;

		mDevice->CreateBuffer(&descGPUBuffer, &InitData, &mSrcDataGPUBuffer);

		D3D11_SHADER_RESOURCE_VIEW_DESC descView;
		ZeroMemory(&descView, sizeof(descView));
		descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descView.BufferEx.FirstElement = 0;
		descView.Format = DXGI_FORMAT_UNKNOWN;
		descView.BufferEx.NumElements = numSpheresX*numSpheresY;

		mDevice->CreateShaderResourceView(mSrcDataGPUBuffer, &descView, &mSrcDataGPUBufferView);
	}

}
#pragma endregion 

#pragma region Render
void Level00::RenderWalls()
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

void Level00::RenderLightCircles()
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

	//FIXME player stuff
	//Node* newPlayerNode = mGrid.GetNodeAt(mExplorers.mTransform->GetPosition());
	if (true/* || mPlayerNode != newPlayerNode*/) {
		//mPlayerNode = newPlayerNode;

		InitializeGrid();

		//Compute 
		mDeviceContext->CSSetShader(mShadowGridComputeShader, NULL, 0);
		mDeviceContext->UpdateSubresource(mSrcDataGPUBuffer, 0, NULL, &mGridNodeData, 0, 0);
		mDeviceContext->CSSetShaderResources(0, 1, &mSrcDataGPUBufferView);
		mDeviceContext->CSSetShaderResources(1, 1, &mShadowsFinalSRV);
		mDeviceContext->CSSetShaderResources(2, 1, &mShadowCastersSRV);
		mDeviceContext->CSSetUnorderedAccessViews(0, 1, &mDestDataGPUBufferView, NULL);
		mDeviceContext->CSSetConstantBuffers(0, 1, &mQuadShaderBuffer);
		for (int i = 0; i < 100; i++) {
			mDeviceContext->Dispatch(34, 1, 1);
			mDeviceContext->CopyResource(mSrcDataGPUBuffer, mDestDataGPUBuffer);
		}
		mDeviceContext->CSSetShader(NULL, NULL, 0);
		mDeviceContext->CSSetShaderResources(0, 3, nullSRV);

		//Copy results to a CPU friendly buffer
		mDeviceContext->CopyResource(mDestDataGPUBufferCPURead, mDestDataGPUBuffer);

		//Map and update
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mDeviceContext->Map(mDestDataGPUBufferCPURead, 0, D3D11_MAP_READ, 0, &mappedResource);
		GridNode* ints = reinterpret_cast<GridNode*>(mappedResource.pData);
		auto g = mGrid.pathFinder.graph.grid;
		for (int i = 0; i < numSpheresX; i++)
		{
			for (int j = 0; j < numSpheresY; j++)
			{
				g[i][j].hasLight = ints[i + j*numSpheresX].hasLight;
				g[i][j].weight = ints[i + j*numSpheresX].weight;

			}
		}
		mDeviceContext->Unmap(mDestDataGPUBufferCPURead, 0);
	}
}

void Level00::RenderShadowMask() {
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

void Level00::RenderExplorer()
{
	const UINT stride = sizeof(mat4f);
	const UINT offset = 0;
	mDeviceContext->IASetInputLayout(mQuadInputLayout);
	mDeviceContext->VSSetShader(mQuadVertexShader, NULL, 0);
	mDeviceContext->PSSetShader(mQuadPixelShader, NULL, 0);

	mQuadShaderData.Color = gPlayerColor;
	mDeviceContext->UpdateSubresource(mQuadShaderBuffer, 0, NULL, &mQuadShaderData, 0, 0);
	mDeviceContext->VSSetConstantBuffers(0, 1, &mQuadShaderBuffer);

	mRenderer->VBindMesh(mWallMesh);
	mDeviceContext->IASetVertexBuffers(1, 1, &mPlayerInstanceBuffer, &stride, &offset);

	mDeviceContext->DrawIndexedInstanced(mWallMesh->GetIndexCount(), mExplorersCount, 0, 0, 0);
}

void Level00::RenderRobots()
{
	for (auto robot : mRobots)
	{
		auto pos = robot.Transform.GetPosition();

		TRACE_SMALL_DIAMOND(pos, Colors::red);
		TRACE_SMALL_CROSS(pos, Colors::red);

		/*for (int i = 1, len = robot.Waypoints.size(); i < len; i++)
		{
			TRACE_LINE(robot.Waypoints[i - 1], robot.Waypoints[i], Colors::yellow);
		}*/
	}
}

void Level00::RenderGrid()
{
	int i, j;
	for (i = 0; i < numSpheresX; i++)
	{
		for (j = 0; j < numSpheresY; j++)
		{
			auto pos = mGrid.graph.grid[i][j].worldPos;
			auto hasLight = mGrid.graph.grid[i][j].hasLight;
			auto weight = mGrid.graph.grid[i][j].weight;

			vec4f c;
			switch ((int)weight) {
			case -10:
				c = Colors::magenta;
				break;
			case -2:
				c = Colors::red;
				break;
			case -1:
				c = Colors::yellow;
				break;
			case 0:
				c = Colors::green;
				break;
			default:
				c = vec4f(0, 0, weight*0.01f, 1);
				break;
			}
			//TRACE_BOX(pos, hasLight ? Colors::cyan : Colors::magenta);
			TRACE_SMALL_BOX(pos, c);
		}
	}
}

#pragma endregion

#pragma region Update
void Level00::UpdateExplorer() {
	mat4f explorerWorldMatrix[MAX_CLIENTS];

	int c = 0;
	for each(auto explorer in mExplorer) {
		if (explorer->mNetworkID->mIsActive) {
			explorerWorldMatrix[c] = explorer->mTransform->GetWorldMatrix().transpose();
			c++;
		}
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	mDeviceContext->Map(mPlayerInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Copy the instances array into the instance buffer.
	memcpy(mappedResource.pData, &explorerWorldMatrix, mExplorersCount*sizeof(mat4f));
	// Unlock the instance buffer.
	mDeviceContext->Unmap(mPlayerInstanceBuffer, 0);
}

void Level00::UpdateGrid()
{
	for (size_t x = 0; x < numSpheresX; x++)
	{
		for (size_t y = 0; y < numSpheresY; y++)
		{
			auto node = &mGrid.graph.grid[x][y];

			//node->weight = node->hasLight ? 1 : FLT_MAX;

			//RayCastHit<vec3f> hit;
			//if (RayCast(&hit, { node->position, vec3f(0, 0, 1) }, mWallColliders, mWallCount))
			//{
			//	node->weight = FLT_MAX;
			//}
			vec4f color;
			switch ((int)node->weight)
			{
			case -10:
				color = { 0.0f, 0.0f, 0.0f, 0.5f };// Colors::black;
				break;
			case -2:
				color = { 1.0f, 0.0f, 0.0f, 0.5f };// Colors::red;
				break;
			case -1:
				color = { 1.0f, 1.0f, 0.0f, 0.5f };// Colors::yellow;
				break;
			case 0:
				color = { 0.0f, 0.0f, 0.0f, 0.5f };// Colors::green;
				break;
			default:
				auto c = node->weight / 50;
				c = c > 1 ? 1 : c < 0 ? 0 : c;
				auto g = 1 - (c > .5f ? c - .5f : 1 - c - .5f);
				color = Vector4(c, 1 - c, g, 0.5f);
				break;
			}

			TRACE_SMALL_BOX(node->worldPos, color);
		}
	}

}

void Level00::UpdateRobots()
{
	if (!mExplorer[0]->mTransform) return;
	auto player = mExplorer[0]->mTransform;

	for (int i = 0; i < mRobotCount; i++)
	{
		auto robot = &mRobots[i];
		auto follower = mFollowers[i];

		auto n = mGrid.GetNodeAt(robot->Transform.GetPosition());

		if (n->weight == -2)
		{
			auto list = mGrid.graph.GetNodeConnections(n);
			Node* minNode = n;
			float minWeight = FLT_MAX;

			for (auto conn : *list) {
				float w = conn.to->weight;
				if (w < 0) continue;
				if (w < minWeight)
				{
					minNode = conn.to;
					minWeight = w;
				}
			}
			n = minNode;

			delete list;
		}

		SearchResult<Node> searchResult;
		searchResult.path.push_back(n);

		TRACE_DIAMOND(n->worldPos, Colors::green);

		if (n->weight > 100 || n->weight < 0) continue;

		while (n->weight > 0) {
			auto list = mGrid.graph.GetNodeConnections(n);
			Node* minNode = n;
			float connCost = 0;

			for (auto conn : *list) {
				float w = conn.to->weight;
				if (w < 0) continue;
				if (w < minNode->weight)
				{
					minNode = conn.to;
					connCost = conn.cost;
				}
			}
			TRACE_LINE(n->worldPos, minNode->worldPos, Colors::yellow);
			n = minNode;

			searchResult.path.push_back(n);

			delete list;
		}

		if (searchResult.path.size() <= 1)
		{
			continue;
		}

		follower.MoveTowards(*player, searchResult);

		//mRobotTransforms[i] = robot->Transform.GetWorldMatrix();
	}
}

#pragma endregion 

#pragma region Initialization

void Level00::InitializeCamera()
{
	Options& opt = Application::SharedInstance().mOptions;
	float aspectRatio = static_cast<float>(opt.mWindowWidth) / opt.mWindowHeight;
	float halfHeight = 33.5f;
	float halfWidth = 33.5f * aspectRatio;
	mProjectionMatrix = mat4f::normalizedOrthographicLH(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.3f, 1000.0f).transpose();
	mViewMatrix = mat4f::lookAtLH(vec3f(0.0f, 0.0f, 1.0f), vec3f(0.0f, 0.0f, -30.0f), vec3f(0.0f, 1.0f, 0.0f)).transpose();

	mQuadShaderData.View = mViewMatrix;
	mQuadShaderData.Projection = mProjectionMatrix;

	TRACE_SET_VIEW_PROJ(mViewMatrix, mProjectionMatrix)
}

void Level00::InitializeLevel()
{
	LevelReader levelReader("Assets/UnitySceneExport.json");
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
	mRobotCount = mRobots.size();
	mRobotTransforms = reinterpret_cast<mat4f*>(mSceneAllocator.Allocate(sizeof(mat4f) * mRobotCount, alignof(mat4f), 0));

	// SpawnPoint
	mSpawnPoint.mTransform = &mSpawnPointTransform;
	mSpawnPoint.mTransform->SetPosition(levelReader.mPlayerPos + vec3f(45, -20, 0));
	
	// Disable explorers
	mExplorersCount = 0;
	for each(auto &explorer in mExplorer) {
		explorer->mNetworkID->mIsActive = false;
		explorer->mNetworkID->mHasAuthority = false;
	}

	// Goal
	mGoal.mTransform = &mGoalTransform;
	mGoal.mTransform->SetPosition(levelReader.mGoalPos);
	mGoal.mTransform->RotateYaw(PI);
}

void Level00::SpawnNewExplorer(int id) {
	mExplorer[id]->mTransform = &mExplorerTransform[id];
	mExplorer[id]->mTransform->SetPosition(mSpawnPoint.mTransform->GetPosition());
				 
	mExplorer[id]->mBoxCollider = &mPlayerCollider;
	mExplorer[id]->mBoxCollider->origin = mExplorerTransform[id].GetPosition();
	mExplorer[id]->mBoxCollider->halfSize = vec3f(UNITY_QUAD_RADIUS) * mExplorerTransform[id].GetScale();
				 
	mExplorer[id]->mNetworkID->mIsActive = true;
	mExplorer[id]->mNetworkID->mUUID = MyUUID::GenUUID();
	mExplorersCount++;

	if (mNetworkManager->mMode == NetworkManager::Mode::SERVER) {
		Packet p(PacketTypes::SPAWN_EXPLORER);
		p.UUID = mExplorer[id]->mNetworkID->mUUID;
		p.ClientID = id;
		mNetworkManager->mServer.SendToAll(&p);

		Packet p2(PacketTypes::GRANT_AUTHORITY);
		p2.UUID = mExplorer[id]->mNetworkID->mUUID;
		mNetworkManager->mServer.Send(id, &p2);
	}
}

void Level00::SpawnExistingExplorer(int id, int UUID) {
	mExplorer[id]->mTransform = &mExplorerTransform[id];
	mExplorer[id]->mTransform->SetPosition(mSpawnPoint.mTransform->GetPosition());

	mExplorer[id]->mBoxCollider = &mPlayerCollider;
	mExplorer[id]->mBoxCollider->origin = mExplorerTransform[id].GetPosition();
	mExplorer[id]->mBoxCollider->halfSize = vec3f(UNITY_QUAD_RADIUS) * mExplorerTransform[id].GetScale();

	mExplorer[id]->mNetworkID->mIsActive = true;
	mExplorer[id]->mNetworkID->mUUID = UUID;
	mExplorersCount++;
}


void Level00::GrantAuthority(int UUID) {
	// Player
	for each(auto &explorer in mExplorer) {
		if (explorer->mNetworkID->mUUID == UUID)
		explorer->mNetworkID->mHasAuthority = true;
	}
}

void Level00::SyncTransform(int UUID, vec3f pos)
{
	// Player
	for each(auto &explorer in mExplorer) {
		if (explorer->mNetworkID->mUUID == UUID) {
			explorer->mTransform->SetPosition(pos);
			explorer->mBoxCollider->origin = pos;
		}
	}
}

void Level00::InitializeGrid()
{
	auto grid = mGrid.pathFinder.graph.grid;


	for (int i = 0; i < numSpheresX; i++) {
		for (int j = 0; j < numSpheresY; j++) {
			grid[i][j].weight = -10;
		}
	}
	for each(auto explorer in mExplorer) {
		if (explorer->mNetworkID->mIsActive)
			mGrid.GetNodeAt(explorer->mTransform->GetPosition())->weight = 0;
	}
	

	for (int i = 0; i < numSpheresX; i++) {
		for (int j = 0; j < numSpheresY; j++) {
			mGridNodeData[i + j*numSpheresX].worldPos = grid[i][j].worldPos;
			mGridNodeData[i + j*numSpheresX].x = grid[i][j].x;
			mGridNodeData[i + j*numSpheresX].y = grid[i][j].y;
			mGridNodeData[i + j*numSpheresX].weight = grid[i][j].weight;
		}
	}
}

void Level00::InitializeRobots()
{

	mFollowers = reinterpret_cast<TargetFollower*>(mSceneAllocator.Allocate(sizeof(TargetFollower) * mRobotCount, alignof(TargetFollower), 0));
	for (auto i = 0; i < mRobotCount; i++)
	{
		new (&mFollowers[i]) TargetFollower(mRobots[i].Transform, mAABBs, mAABBCount);
	}
}

void Level00::LoadTransforms(mat4f** transforms, vec3f* positions, vec3f* rotations, vec3f* scales, int size, int TransformType)
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

void Level00::LoadStaticSceneObjects(LegacySceneObject** sceneObjects, mat4f** transforms, BoxCollider** colliders, vec3f* positions, vec3f* rotations, vec3f* scales, int size)
{
	// Allocate size SceneObjects
	*sceneObjects = reinterpret_cast<LegacySceneObject*>(mSceneAllocator.Allocate(sizeof(LegacySceneObject) * size, alignof(BaseSceneObject), 0));
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

void Level00::LoadLights(LegacySceneObject** sceneObjects, mat4f** transforms, SphereCollider** colliders, vec3f* positions, int size)
{
	// Allocate size SceneObjects
	*sceneObjects = reinterpret_cast<LegacySceneObject*>(mSceneAllocator.Allocate(sizeof(LegacySceneObject) * size, alignof(LegacySceneObject), 0));
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

void Level00::SetAABBs(RectInfo rectInfo, AABB<Vector2>* aabb, int offset)
{
	for (size_t i = 0; i < rectInfo.Position.size(); i++)
	{
		aabb[i + offset].origin = rectInfo.Position[i];
		aabb[i + offset].halfSize = rectInfo.Scale[i];
	}
}

void Level00::InitializeGeometry()
{
	InitializeQuadMesh();
	InitializeCircleMesh();
}

void Level00::InitializeQuadMesh()
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

void Level00::InitializeCircleMesh()
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

void Level00::InitializeWallShaders()
{
	D3D11_INPUT_ELEMENT_DESC inputDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	// Create the shader on the device
	mDevice->CreateVertexShader(
		gQuadVertexShader,
		sizeof(gQuadVertexShader),
		NULL,
		&mQuadVertexShader);


	// Before cleaning up the data, create the input layout
	if (inputDescription) {
		mDevice->CreateInputLayout(
			inputDescription,					// Reference to Description
			5,									// Number of elments inside of Description
			gQuadVertexShader,
			sizeof(gQuadVertexShader),
			&mQuadInputLayout);
	}

	// Create the shader on the device
	mDevice->CreatePixelShader(
		gQuadPixelShader,
		sizeof(gQuadPixelShader),
		NULL,
		&mQuadPixelShader);

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

	mDevice->CreateVertexShader(gBillboardVertexShader, sizeof(gBillboardVertexShader), nullptr, &mBillboardVertexShader);

	mDevice->CreatePixelShader(gBillboardPixelShader, sizeof(gBillboardPixelShader), nullptr, &mBillboardPixelShader);

	mDevice->CreateComputeShader(gShadowGridComputeShader, sizeof(gShadowGridComputeShader), nullptr, &mShadowGridComputeShader);

	D3D11_BUFFER_DESC pointDesc;
	pointDesc.ByteWidth = sizeof(PointShaderData);
	pointDesc.Usage = D3D11_USAGE_DEFAULT;
	pointDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pointDesc.CPUAccessFlags = 0;
	pointDesc.MiscFlags = 0;
	pointDesc.StructureByteStride = 0;

	mDevice->CreateBuffer(&pointDesc, NULL, &mPointShaderBuffer);

	mDevice->CreatePixelShader(gShadowPixelShader, sizeof(gShadowPixelShader), nullptr, &mShadowPixelShader);

	mDevice->CreatePixelShader(gShadowCasterPixelShader, sizeof(gShadowCasterPixelShader), nullptr, &mShadowCasterPixelShader);

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

void Level00::InitializeLightShaders()
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

	// Create the shader on the device
	mDevice->CreateVertexShader(
		gCircleVertexShader,
		sizeof(gCircleVertexShader),
		NULL,
		&mCircleVertexShader);

	// Before cleaning up the data, create the input layout
	if (inputDescription) {
		mDevice->CreateInputLayout(
			inputDescription,					// Reference to Description
			6,									// Number of elments inside of Description
			gCircleVertexShader,
			sizeof(gCircleVertexShader),
			&mCircleInputLayout);
	}

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

void Level00::InitializePlayerShaders() {
	//Mostly, re-using Walls shaders for now (or forever)
	// Instance buffer
	D3D11_BUFFER_DESC playerInstanceBufferDesc;
	playerInstanceBufferDesc.ByteWidth = MAX_CLIENTS * sizeof(mat4f);
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

void Level00::HandleInput(Input& input)
{
	// Player Mouse

	if (input.GetKeyDown(KEYCODE_1))
	{
		Application::SharedInstance().LoadScene<MainMenuScene>();
	}

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
			mCircleColorWeights[hit.index] = mCircleColorWeights[hit.index] > 0 ? 0.0f : 1.0f;
		}
	}

	// Player Movement
	float mPlayerSpeed = 0.25f;
	//FIXME
	for each(auto explorer in mExplorer) {
		if (explorer->mNetworkID->mHasAuthority && explorer->mTransform) {
			auto pos = explorer->mTransform->GetPosition();
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

			BoxCollider aabb = { pos, explorer->mBoxCollider->halfSize };
			bool canMove = true;
			for (int i = 0; i < mWallCount; i++)
			{
				if (IntersectAABBAABB(aabb, mWallColliders[i]))
				{
					canMove = false;
					break;
				}
			}

			if (canMove) {
				explorer->mTransform->SetPosition(pos);
				explorer->mBoxCollider->origin = pos;

				Packet p(PacketTypes::SYNC_TRANSFORM);
				p.UUID = explorer->mNetworkID->mUUID;
				p.Position = pos;
				mNetworkManager->mClient.SendData(&p);
			}
		}
	}
	UpdateExplorer();
}
