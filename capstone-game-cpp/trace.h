#pragma once

#if defined _DEBUG
#include <sstream>
#include <string>
#include <Windows.h>
#include <debugapi.h>
#include <functional>
#include <GraphicsMath/Vector.hpp>

typedef std::function<void(const float&, const float&, const float&, const float&, const float&, const float&, const cliqCity::graphicsMath::Vector4& color)> __TraceFunction;
extern __TraceFunction __gTraceLine;

class Trace
{
	std::stringstream ss;
	char buff[128];
public:
	static Trace &GetTrace() { static Trace trace; return trace; }
	Trace &operator << (int value) { ss << value; return *this; }
	Trace &operator << (short value) { ss << value; return *this; }
	Trace &operator << (float value) { ss << value; return *this; }
	Trace &operator << (long value) { ss << value; return *this; }
	Trace &operator << (bool value) { ss << value; return *this; }
	Trace &operator << (char* value) { ss << value; return *this; }
	Trace &operator << (std::string value) { ss << value; return *this; }
	Trace &operator << (Trace &(*function)(Trace &trace)) { return function(*this); }

	static Trace &endl(Trace &trace)
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

			//OutputDebugString(str.c_str());
			//OutputDebugString("\n");
		}

		trace.ss.clear();

		return trace;
	}
};

inline void __TraceBox(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(xmax, ymax, z, xmin, ymax, z, color);
	__gTraceLine(xmin, ymin, z, xmax, ymin, z, color);
	__gTraceLine(xmax, ymax, z, xmax, ymin, z, color);
	__gTraceLine(xmin, ymin, z, xmin, ymax, z, color);
}

inline void __TraceSmallBox(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(xmax, ymax, z, xmin, ymax, z, color);
	__gTraceLine(xmin, ymin, z, xmax, ymin, z, color);
	__gTraceLine(xmax, ymax, z, xmax, ymin, z, color);
	__gTraceLine(xmin, ymin, z, xmin, ymax, z, color);
}

inline void __TraceXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(xmin, ymin, z, xmax, ymax, z, color);
	__gTraceLine(xmin, ymax, z, xmax, ymin, z, color);
}

inline void __TraceSmallXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(xmin, ymin, z, xmax, ymax, z, color);
	__gTraceLine(xmin, ymax, z, xmax, ymin, z, color);
}

inline void __TraceCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(x, ymin, z, x, ymax, z, color);
	__gTraceLine(xmin, y, z, xmax, y, z, color);
}

inline void __TraceSmallCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(x, ymin, z, x, ymax, z, color);
	__gTraceLine(xmin, y, z, xmax, y, z, color);
}

inline void __TraceDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(xmin, y,    z,   x,    ymax, z, color);
	__gTraceLine(x,    ymax, z,   xmax, y,    z, color);
	__gTraceLine(xmax, y,    z,   x,    ymin, z, color);
	__gTraceLine(x,    ymin, z,   xmin, y,    z, color);
}

inline void __TraceSmallDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
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

	__gTraceLine(xmin, y,    z,   x,    ymax, z, color);
	__gTraceLine(x,    ymax, z,   xmax, y,    z, color);
	__gTraceLine(xmax, y,    z,   x,    ymin, z, color);
	__gTraceLine(x,    ymin, z,   xmin, y,    z, color);
}

inline void __TraceLine(const cliqCity::graphicsMath::Vector3& from, const cliqCity::graphicsMath::Vector3& to, const cliqCity::graphicsMath::Vector4& color)
{
	__gTraceLine(from.x, from.y, from.z, to.x, to.y, to.z, color);
}

#define TRACE(message)							Trace::GetTrace () << message << Trace::endl
#define TRACE_LINE(from, to, color)				__TraceLine			(from, to, color)
#define TRACE_BOX(position, color)				__TraceBox			(position, color)
#define TRACE_DIAMOND(position, color)			__TraceDiamond		(position, color)
#define TRACE_CROSS(position, color)			__TraceCross		(position, color)
#define TRACE_XCROSS(position, color)			__TraceXCross		(position, color)
#define TRACE_SMALL_BOX(position, color)		__TraceSmallBox		(position, color)
#define TRACE_SMALL_DIAMOND(position, color)	__TraceSmallDiamond	(position, color)
#define TRACE_SMALL_CROSS(position, color)		__TraceSmallCross	(position, color)
#define TRACE_SMALL_XCROSS(position, color)		__TraceSmallXCross	(position, color)

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

#endif