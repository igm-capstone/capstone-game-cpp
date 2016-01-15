#pragma once
#if defined _DEBUG

#include <sstream>

#define TRACE(message)							Trace::GetTrace() << message << Trace::endl
#define TRACE_LINE(from, to, color)				Trace::TraceLine			(from, to, color)
#define TRACE_BOX(position, color)				Trace::TraceBox			(position, color)
#define TRACE_DIAMOND(position, color)			Trace::TraceDiamond		(position, color)
#define TRACE_CROSS(position, color)			Trace::TraceCross		(position, color)
#define TRACE_XCROSS(position, color)			Trace::TraceXCross		(position, color)
#define TRACE_SMALL_BOX(position, color)		Trace::TraceSmallBox		(position, color)
#define TRACE_SMALL_DIAMOND(position, color)	Trace::TraceSmallDiamond	(position, color)
#define TRACE_SMALL_CROSS(position, color)		Trace::TraceSmallCross	(position, color)
#define TRACE_SMALL_XCROSS(position, color)		Trace::TraceSmallXCross	(position, color)

#define RENDER_TRACE()							Trace::GetTrace().Render();

#define TRACE_SET_VIEW_PROJ(mViewMatrix, mProjectionMatrix)		Trace::GetTrace().mLineTraceShaderData = { mViewMatrix, mProjectionMatrix };

typedef std::function<void(const float&, const float&, const float&, const float&, const float&, const float&, const cliqCity::graphicsMath::Vector4& color)> __TraceFunction;

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

class Trace
{
private:
	static const int gLineTraceMaxCount = 20000;
	static const int gLineTraceVertexCount = 2 * gLineTraceMaxCount;

	static const int gTraceMemorySize = 10024 + (gLineTraceVertexCount * 8 * 4);
	static char gTraceMemory[gTraceMemorySize];

	static const int gMeshMemorySize = 1024;
	static char gStaticMeshMemory[gMeshMemorySize];

	LinearAllocator					mTraceAllocator;
	LinearAllocator					mStaticMeshAllocator;
	MeshLibrary<LinearAllocator>	mStaticMeshLibrary;

	struct LineTraceVertex
	{
		vec4f Color;
		vec3f Position;
	};
	struct LineTraceShaderData
	{
		mat4f View;
		mat4f Projection;
	};

	std::stringstream ss;
	char buff[128];

	Renderer*						mRenderer;
	ID3D11Device*					mDevice;
	ID3D11DeviceContext*			mDeviceContext;
	LineTraceVertex*				mLineTraceVertices;
	IMesh*							mLineTraceMesh;
	ID3D11InputLayout*				mLineTraceInputLayout;
	ID3D11VertexShader*				mLineTraceVertexShader;
	ID3D11PixelShader*				mLineTracePixelShader;
	ID3D11Buffer*					mLineTraceShaderBuffer;

	int								mLineTraceDrawCount;
	bool							mLineTraceOverflow;

	Trace();
	~Trace();
	void InitializeLineTraceMesh();
	void InitializeLineTraceShaders();

public:
	void TraceLine(const float& from_x, const float& from_y, const float& from_z, const float& to_x, const float& to_y, const float& to_z, const vec4f& color);

	LineTraceShaderData	mLineTraceShaderData;

	static Trace &GetTrace() {
		static Trace trace;
		return trace;
	}
	void Render();

	Trace &operator << (int value) { ss << value; return *this; }
	Trace &operator << (short value) { ss << value; return *this; }
	Trace &operator << (float value) { ss << value; return *this; }
	Trace &operator << (long value) { ss << value; return *this; }
	Trace &operator << (bool value) { ss << value; return *this; }
	Trace &operator << (char* value) { ss << value; return *this; }
	Trace &operator << (std::string value) { ss << value; return *this; }
	Trace &operator << (Trace &(*function)(Trace &trace)) { return function(*this); }

	static Trace &endl(Trace &trace);

	static void TraceBox(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceSmallBox(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceSmallXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceSmallCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceSmallDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color);

	static void TraceLine(const cliqCity::graphicsMath::Vector3& from, const cliqCity::graphicsMath::Vector3& to, const cliqCity::graphicsMath::Vector4& color);

};



#else

#define TRACE(message)
#define TRACE_LINE(from, to, color)
#define TRACE_BOX(position, color)
#define TRACE_DIAMOND(position, color)
#define TRACE_CROSS(position, color)
#define TRACE_XCROSS(position, color)
#define TRACE_SMALL_BOX(position, color)
#define TRACE_SMALL_DIAMOND(position, color)
#define TRACE_SMALL_CROSS(position, color)
#define TRACE_SMALL_XCROSS(position, color)

#define RENDER_TRACE()

#define TRACE_SET_VIEW_PROJ(mViewMatrix, mProjectionMatrix)	
#endif