#include <stdafx.h>
#if defined _DEBUG
#include <trace.h>

#include <string>
#include <debugapi.h>
#include <functional>
#include <GraphicsMath/Vector.hpp>
#include "Rig3D/GraphicsMath/cgm.h"
#include "Rig3D/Intersection.h"
#include "Rig3D/Parametric.h"
#include "Rig3D/IApplication.h"
#include "Rig3D\Engine.h"

#include "Memory\Memory\LinearAllocator.h"

#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Graphics\MeshLibrary.h"

#include "Rig3D\Common\Transform.h"

#include "Shaders/obj/LineTracePixelShader.h"
#include "Shaders/obj/LineTraceVertexShader.h"

char Trace::gTraceMemory[Trace::gTraceMemorySize] = { 0 };
char Trace::gStaticMeshMemory[Trace::gMeshMemorySize] = { 0 };

Trace::Trace() :
	mTraceAllocator(static_cast<void*>(gTraceMemory), static_cast<void*>(gTraceMemory + gTraceMemorySize)), 
	mStaticMeshAllocator(static_cast<void*>(gStaticMeshMemory), static_cast<void*>(gStaticMeshMemory + gMeshMemorySize))
{
	mStaticMeshLibrary.SetAllocator(&mStaticMeshAllocator);

	auto mEngine = &Singleton<Engine>::SharedInstance();
	mRenderer = mEngine->GetRenderer();
	mDeviceContext = mRenderer->GetDeviceContext();
	mDevice = mRenderer->GetDevice();

	InitializeLineTraceMesh();
	InitializeLineTraceShaders();
}

Trace::~Trace() {
	mLineTraceMesh->~IMesh();

	ReleaseMacro(mLineTraceInputLayout);
	ReleaseMacro(mLineTraceVertexShader);
	ReleaseMacro(mLineTracePixelShader);
	ReleaseMacro(mLineTraceShaderBuffer);
}

void Trace::InitializeLineTraceMesh()
{

	mLineTraceVertices = reinterpret_cast<LineTraceVertex*>(mTraceAllocator.Allocate(sizeof(LineTraceVertex) * gLineTraceVertexCount, alignof(LineTraceVertex), 0));

	uint16_t lineTraceIndices[gLineTraceVertexCount];
	for (auto i = 0; i < gLineTraceVertexCount; i++)
	{
		lineTraceIndices[i] = i;
	}

	mStaticMeshLibrary.NewMesh(&mLineTraceMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mLineTraceMesh, mLineTraceVertices, sizeof(LineTraceVertex) * gLineTraceVertexCount, sizeof(LineTraceVertex));
	mRenderer->VSetDynamicMeshIndexBuffer(mLineTraceMesh, lineTraceIndices, gLineTraceVertexCount);
}

void Trace::InitializeLineTraceShaders() {
	D3D11_INPUT_ELEMENT_DESC inputDescription[] =
	{
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the shader on the device
	mDevice->CreateVertexShader(
		gLineTraceVertexShader,
		sizeof(gLineTraceVertexShader),
		nullptr,
		&mLineTraceVertexShader);

	// Before cleaning up the data, create the input layout
	if (inputDescription) {
		mDevice->CreateInputLayout(
			inputDescription,					// Reference to Description
			2,									// Number of elments inside of Description
			gLineTraceVertexShader,
			sizeof(gLineTraceVertexShader),
			&mLineTraceInputLayout);
	}


	// Create the shader on the device
	mDevice->CreatePixelShader(
		gLineTracePixelShader,
		sizeof(gLineTracePixelShader),
		nullptr,
		&mLineTracePixelShader);


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

void Trace::TraceLine(const float& from_x, const float& from_y, const float& from_z, const float& to_x, const float& to_y, const float& to_z, const vec4f& color) {
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
	mLineTraceVertices[index].Position.x = from_x;
	mLineTraceVertices[index].Position.y = from_y;
	mLineTraceVertices[index].Position.z = from_z;
	mLineTraceVertices[index].Color = color;

	index = mLineTraceDrawCount++;
	mLineTraceVertices[index].Position.x = to_x;
	mLineTraceVertices[index].Position.y = to_y;
	mLineTraceVertices[index].Position.z = to_z;
	mLineTraceVertices[index].Color = color;
};

void Trace::Render() {
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
};

Trace& Trace::endl(Trace &trace)
{
	trace.ss << std::endl;

	while (true)
	{
		std::string str;
		std::getline(trace.ss, str);

		if (str.empty())
		{
			break;
		}
		
		std::wstring wstr = std::wstring(str.begin(), str.end());
		OutputDebugString(wstr.c_str());
		OutputDebugString(L"\n");
	}

	trace.ss.clear();

	return trace;
}

void Trace::TraceBox(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto pone = cliqCity::graphicsMath::Vector3( 1.0f, 1.0f, 0);
	//const auto none = cliqCity::graphicsMath::Vector3(-1.0f, 1.0f, 0);

	// box
	//__gTraceLine(pos + pone, pos + none, color);
	//__gTraceLine(pos - pone, pos - none, color);
	//__gTraceLine(pos + pone, pos - none, color);
	//__gTraceLine(pos - pone, pos + none, color);

	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(xmax, ymax, z, xmin, ymax, z, color);
	Trace::GetTrace().TraceLine(xmin, ymin, z, xmax, ymin, z, color);
	Trace::GetTrace().TraceLine(xmax, ymax, z, xmax, ymin, z, color);
	Trace::GetTrace().TraceLine(xmin, ymin, z, xmin, ymax, z, color);
}

void Trace::TraceSmallBox(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto pone = cliqCity::graphicsMath::Vector3( .5f, .5f, 0);
	//const auto none = cliqCity::graphicsMath::Vector3(-.5f, .5f, 0);

	//// box
	//__gTraceLine(pos + pone, pos + none, color);
	//__gTraceLine(pos - pone, pos - none, color);
	//__gTraceLine(pos + pone, pos - none, color);
	//__gTraceLine(pos - pone, pos + none, color);

	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(xmax, ymax, z, xmin, ymax, z, color);
	Trace::GetTrace().TraceLine(xmin, ymin, z, xmax, ymin, z, color);
	Trace::GetTrace().TraceLine(xmax, ymax, z, xmax, ymin, z, color);
	Trace::GetTrace().TraceLine(xmin, ymin, z, xmin, ymax, z, color);
}

void Trace::TraceXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto pone = cliqCity::graphicsMath::Vector3( 1.0f, 1.0f, 0);
	//const auto none = cliqCity::graphicsMath::Vector3(-1.0f, 1.0f, 0);

	//// cross
	//__gTraceLine(pos + pone, pos - pone, color);
	//__gTraceLine(pos + none, pos - none, color);

	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(xmin, ymin, z, xmax, ymax, z, color);
	Trace::GetTrace().TraceLine(xmin, ymax, z, xmax, ymin, z, color);
}

void Trace::TraceSmallXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto pone = cliqCity::graphicsMath::Vector3( .5f, .5f, 0);
	//const auto none = cliqCity::graphicsMath::Vector3(-.5f, .5f, 0);

	//// cross
	//__gTraceLine(pos + pone, pos - pone, color);
	//__gTraceLine(pos + none, pos - none, color);

	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(xmin, ymin, z, xmax, ymax, z, color);
	Trace::GetTrace().TraceLine(xmin, ymax, z, xmax, ymin, z, color);
}

void Trace::TraceCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto hone = cliqCity::graphicsMath::Vector3(1.0f, 0.0f, 0);
	//const auto vone = cliqCity::graphicsMath::Vector3(0.0f, 1.0f, 0);

	//// cross
	//__gTraceLine(pos + vone, pos - vone, color);
	//__gTraceLine(pos + hone, pos - hone, color);

	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(x, ymin, z, x, ymax, z, color);
	Trace::GetTrace().TraceLine(xmin, y, z, xmax, y, z, color);
}

void Trace::TraceSmallCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto hone = cliqCity::graphicsMath::Vector3(1.0f, 0.0f, 0);
	//const auto vone = cliqCity::graphicsMath::Vector3(0.0f, 1.0f, 0);

	//// cross
	//__gTraceLine(pos + vone, pos - vone, color);
	//__gTraceLine(pos + hone, pos - hone, color);

	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(x, ymin, z, x, ymax, z, color);
	Trace::GetTrace().TraceLine(xmin, y, z, xmax, y, z, color);
}

void Trace::TraceDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto hone = cliqCity::graphicsMath::Vector3(1.0f, 0.0f, 0);
	//const auto vone = cliqCity::graphicsMath::Vector3(0.0f, 1.0f, 0);

	//// diamond
	//__gTraceLine(pos + vone, pos + hone, color);
	//__gTraceLine(pos + hone, pos - vone, color);
	//__gTraceLine(pos - vone, pos - hone, color);
	//__gTraceLine(pos - hone, pos + vone, color);

	//const auto hone = cliqCity::graphicsMath::Vector3(1.0f, 0.0f, 0);
	//const auto vone = cliqCity::graphicsMath::Vector3(0.0f, 1.0f, 0);

	//// cross
	//__gTraceLine(pos + vone, pos - vone, color);
	//__gTraceLine(pos + hone, pos - hone, color);

	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(xmin, y, z, x, ymax, z, color);
	Trace::GetTrace().TraceLine(x, ymax, z, xmax, y, z, color);
	Trace::GetTrace().TraceLine(xmax, y, z, x, ymin, z, color);
	Trace::GetTrace().TraceLine(x, ymin, z, xmin, y, z, color);
}

void Trace::TraceSmallDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	//const auto hone = cliqCity::graphicsMath::Vector3(.5f,   0, 0);
	//const auto vone = cliqCity::graphicsMath::Vector3(  0, .5f, 0);

	//// diamond
	//__gTraceLine(pos + vone, pos + hone, color);
	//__gTraceLine(pos + hone, pos - vone, color);
	//__gTraceLine(pos - vone, pos - hone, color);
	//__gTraceLine(pos - hone, pos + vone, color);

	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace::GetTrace().TraceLine(xmin, y, z, x, ymax, z, color);
	Trace::GetTrace().TraceLine(x, ymax, z, xmax, y, z, color);
	Trace::GetTrace().TraceLine(xmax, y, z, x, ymin, z, color);
	Trace::GetTrace().TraceLine(x, ymin, z, xmin, y, z, color);
}

void Trace::TraceLine(const cliqCity::graphicsMath::Vector3& from, const cliqCity::graphicsMath::Vector3& to, const cliqCity::graphicsMath::Vector4& color)
{
	Trace::GetTrace().TraceLine(from.x, from.y, from.z, to.x, to.y, to.z, color);
}

#endif