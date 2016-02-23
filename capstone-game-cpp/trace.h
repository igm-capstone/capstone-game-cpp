#pragma once
#include <iomanip>
#if defined _DEBUG

#include <sstream>

#define TRACE(message)                          Trace::GetTrace() << message
#define TRACE_ERROR(message)                    Trace::GetTrace() << "[Error] " << message << Trace::endl
#define TRACE_WARN(message)                     Trace::GetTrace() << "[Warning] " << message << Trace::endl
#define TRACE_LOG(message)                      Trace::GetTrace() << "[Log] " << message << Trace::endl
#define TRACE_LINE(from, to, color)             Trace::TraceLine          (from, to, color)
#define TRACE_BOX(position, color)              Trace::TraceBox           (position, color)
#define TRACE_DIAMOND(position, color)          Trace::TraceDiamond       (position, color)
#define TRACE_CROSS(position, color)            Trace::TraceCross         (position, color)
#define TRACE_XCROSS(position, color)           Trace::TraceXCross        (position, color)
#define TRACE_SMALL_BOX(position, color)        Trace::TraceSmallBox      (position, color)
#define TRACE_SMALL_DIAMOND(position, color)    Trace::TraceSmallDiamond  (position, color)
#define TRACE_SMALL_CROSS(position, color)      Trace::TraceSmallCross    (position, color)
#define TRACE_SMALL_XCROSS(position, color)     Trace::TraceSmallXCross   (position, color)

#define RENDER_TRACE()                          Trace::GetTrace().Render();

typedef std::function<void(const float&, const float&, const float&, const float&, const float&, const float&, const vec4f& color)> __TraceFunction;

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

class Trace
{
private:
	static const int gLineTraceMaxCount = 200000;
	static const int gLineTraceVertexCount = 2 * gLineTraceMaxCount;

	static const int gTraceMemorySize = 1024 + (gLineTraceVertexCount * sizeof(float) * 8);
	static char gTraceMemory[gTraceMemorySize];

	static const int gMeshMemorySize = 32; // Just an IMesh really
	static char gMeshMemory[gMeshMemorySize];

	LinearAllocator					mTraceAllocator;
	LinearAllocator					mStaticMeshAllocator;
	MeshLibrary<LinearAllocator>	mStaticMeshLibrary;

	struct LineTraceVertex
	{
		vec4f Color;
		vec3f Position;
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
	static Trace &GetTrace() {
		static Trace trace;
		return trace;
	}
	void Render();

	Trace &operator << (int value) { ss << value; return *this; }
	Trace &operator << (unsigned int value) { ss << value; return *this; }
	Trace &operator << (short value) { ss << value; return *this; }
	Trace &operator << (float value) { ss << value; return *this; }
	Trace &operator << (double value) { ss << value; return *this; }
	Trace &operator << (long value) { ss << value; return *this; }
	Trace &operator << (bool value) { ss << value; return *this; }
	Trace &operator << (char* value) { ss << value; return *this; }
	Trace &operator << (const char* value) { ss << value; return *this; }
	Trace &operator << (std::string value) { ss << value; return *this; }
	Trace &operator << (Trace &(*function)(Trace &trace)) { return function(*this); }
	Trace &operator << (vec2f v) { ss << std::fixed << std::setprecision(2) << "vec2f(" << v.x << ", " << v.y << ")"; return *this; }
	Trace &operator << (vec3f v) { ss << std::fixed << std::setprecision(2) << "vec3f(" << v.x << ", " << v.y << ", " << v.z << ")"; return *this; }
	Trace &operator << (vec4f v) { ss << std::fixed << std::setprecision(2) << "vec4f(" << v.x << ", " << v.y << ", " << v.z << ", " << v.z << ")"; return *this; }

	static Trace &endl(Trace &trace);

	static void TraceBox(const vec3f& pos, const vec4f& color);
	static void TraceSmallBox(const vec3f& pos, const vec4f& color);
	static void TraceXCross(const vec3f& pos, const vec4f& color);
	static void TraceSmallXCross(const vec3f& pos, const vec4f& color);
	static void TraceCross(const vec3f& pos, const vec4f& color);
	static void TraceSmallCross(const vec3f& pos, const vec4f& color);
	static void TraceDiamond(const vec3f& pos, const vec4f& color);
	static void TraceSmallDiamond(const vec3f& pos, const vec4f& color);
	static void TraceLine(const vec3f& from, const vec3f& to, const vec4f& color);
	void TraceLine(const float& from_x, const float& from_y, const float& from_z, const float& to_x, const float& to_y, const float& to_z, const vec4f& color);
};


#else

#define TRACE(message)
#define TRACE_LINE(from, to, color)
#define TRACE_ERROR(message)
#define TRACE_WARN(message)
#define TRACE_LOG(message)
#define TRACE_BOX(position, color)
#define TRACE_DIAMOND(position, color)
#define TRACE_CROSS(position, color)
#define TRACE_XCROSS(position, color)
#define TRACE_SMALL_BOX(position, color)
#define TRACE_SMALL_DIAMOND(position, color)
#define TRACE_SMALL_CROSS(position, color)
#define TRACE_SMALL_XCROSS(position, color)

#define RENDER_TRACE()

#endif