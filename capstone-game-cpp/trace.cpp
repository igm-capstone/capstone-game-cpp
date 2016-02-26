#include <stdafx.h>
#include "CameraManager.h"
#if defined _DEBUG

#include <trace.h>
#include "Console.h"
#include "Uniforms.h"

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

#include "Shaders/obj/VSFwdLineTrace.h"
#include "Shaders/obj/PSFwdColor.h"

char Trace::gTraceMemory[Trace::gTraceMemorySize] = { 0 };
char Trace::gMeshMemory[Trace::gMeshMemorySize] = { 0 };

Trace::Trace() :
	mTraceAllocator(static_cast<void*>(gTraceMemory), static_cast<void*>(gTraceMemory + gTraceMemorySize)), 
	mStaticMeshAllocator(static_cast<void*>(gMeshMemory), static_cast<void*>(gMeshMemory + gMeshMemorySize))
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
	ReleaseMacro(mVSLineTrace);
	ReleaseMacro(mPSSimpleColor);
	ReleaseMacro(mLineTraceShaderBuffer);
}

void Trace::InitializeLineTraceMesh()
{
	auto lineTraceIndices = reinterpret_cast<uint32_t*>(mTraceAllocator.Allocate(sizeof(uint32_t) * gLineTraceVertexCount, alignof(uint32_t), 0));

	for (auto i = 0; i < gLineTraceVertexCount; i++)
	{
		lineTraceIndices[i] = i;
	}
	
	mTraceAllocator.Free();

	mLineTraceVertices = reinterpret_cast<GPU::Vertex2*>(mTraceAllocator.Allocate(sizeof(GPU::Vertex2) * gLineTraceVertexCount, alignof(GPU::Vertex2), 0));

	mStaticMeshLibrary.NewMesh(&mLineTraceMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mLineTraceMesh, mLineTraceVertices, sizeof(GPU::Vertex2) * gLineTraceVertexCount, sizeof(GPU::Vertex2));
	mRenderer->VSetStaticMeshIndexBuffer(mLineTraceMesh, lineTraceIndices, gLineTraceVertexCount);
}

void Trace::InitializeLineTraceShaders() {
	D3D11_INPUT_ELEMENT_DESC inputDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the shader on the device
	mDevice->CreateVertexShader(
		gVSFwdLineTrace,
		sizeof(gVSFwdLineTrace),
		nullptr,
		&mVSLineTrace);

	// Before cleaning up the data, create the input layout
	if (inputDescription) {
		mDevice->CreateInputLayout(
			inputDescription,					// Reference to Description
			2,									// Number of elments inside of Description
			gVSFwdLineTrace,
			sizeof(gVSFwdLineTrace),
			&mLineTraceInputLayout);
	}


	// Create the shader on the device
	mDevice->CreatePixelShader(
		gPSFwdColor,
		sizeof(gPSFwdColor),
		nullptr,
		&mPSSimpleColor);


	// Constant buffers ----------------------------------------
	D3D11_BUFFER_DESC lineTraceBufferDataDesc;
	lineTraceBufferDataDesc.ByteWidth = sizeof(CBuffer::Camera);
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
	mDeviceContext->VSSetShader(mVSLineTrace, nullptr, 0);
	mDeviceContext->PSSetShader(mPSSimpleColor, nullptr, 0);

	

	mDeviceContext->UpdateSubresource(static_cast<DX11Mesh*>(mLineTraceMesh)->mVertexBuffer, 0, nullptr, mLineTraceVertices, 0, 0);
	mDeviceContext->UpdateSubresource(mLineTraceShaderBuffer, 0, nullptr, Singleton<CameraManager>::SharedInstance().GetCBufferPersp(), 0, 0);
	mDeviceContext->VSSetConstantBuffers(0, 1, &mLineTraceShaderBuffer);

	mRenderer->VBindMesh32(mLineTraceMesh);
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
		
		Console::Log(str.c_str());
		std::wstring wstr = std::wstring(str.begin(), str.end());

		OutputDebugString(wstr.c_str());
		OutputDebugString(L"\n");
	}

	trace.ss.clear();

	return trace;
}

void Trace::TraceBox(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(xmax, ymax, z, xmin, ymax, z, color);
	trace.TraceLine(xmin, ymin, z, xmax, ymin, z, color);
	trace.TraceLine(xmax, ymax, z, xmax, ymin, z, color);
	trace.TraceLine(xmin, ymin, z, xmin, ymax, z, color);
}

void Trace::TraceSmallBox(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 0.75f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(xmax, ymax, z, xmin, ymax, z, color);
	trace.TraceLine(xmin, ymin, z, xmax, ymin, z, color);
	trace.TraceLine(xmax, ymax, z, xmax, ymin, z, color);
	trace.TraceLine(xmin, ymin, z, xmin, ymax, z, color);
}

void Trace::TraceXCross(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(xmin, ymin, z, xmax, ymax, z, color);
	trace.TraceLine(xmin, ymax, z, xmax, ymin, z, color);
}

void Trace::TraceSmallXCross(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(xmin, ymin, z, xmax, ymax, z, color);
	trace.TraceLine(xmin, ymax, z, xmax, ymin, z, color);
}

void Trace::TraceCross(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(x, ymin, z, x, ymax, z, color);
	trace.TraceLine(xmin, y, z, xmax, y, z, color);
}

void Trace::TraceSmallCross(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(x, ymin, z, x, ymax, z, color);
	trace.TraceLine(xmin, y, z, xmax, y, z, color);
}

void Trace::TraceDiamond(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 1.0f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(xmin, y, z, x, ymax, z, color);
	trace.TraceLine(x, ymax, z, xmax, y, z, color);
	trace.TraceLine(xmax, y, z, x, ymin, z, color);
	trace.TraceLine(x, ymin, z, xmin, y, z, color);
}

void Trace::TraceSmallDiamond(const vec3f& pos, const vec4f& color)
{
	const auto halfSize = 0.5f;

	auto xmin = pos.x - halfSize;
	auto xmax = pos.x + halfSize;
	auto ymin = pos.y - halfSize;
	auto ymax = pos.y + halfSize;
	auto x = pos.x;
	auto y = pos.y;
	auto z = pos.z;

	Trace& trace = Trace::GetTrace();
	trace.TraceLine(xmin, y, z, x, ymax, z, color);
	trace.TraceLine(x, ymax, z, xmax, y, z, color);
	trace.TraceLine(xmax, y, z, x, ymin, z, color);
	trace.TraceLine(x, ymin, z, xmin, y, z, color);
}

void Trace::TraceLine(const vec3f& from, const vec3f& to, const vec4f& color)
{
	Trace::GetTrace().TraceLine(from.x, from.y, from.z, to.x, to.y, to.z, color);
}

#endif