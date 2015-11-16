#pragma once

#if defined _DEBUG
#include <sstream>
#include <string>
#include <Windows.h>
#include <debugapi.h>
#include <functional>
#include <GraphicsMath/Vector.hpp>

extern std::function<void(const cliqCity::graphicsMath::Vector3&, const cliqCity::graphicsMath::Vector3&, const cliqCity::graphicsMath::Vector4&)> __gTraceLine;

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
	const auto pone = cliqCity::graphicsMath::Vector3( 1.0f, 1.0f, 0);
	const auto none = cliqCity::graphicsMath::Vector3(-1.0f, 1.0f, 0);

	// box
	__gTraceLine(pos + pone, pos + none, color);
	__gTraceLine(pos - pone, pos - none, color);
	__gTraceLine(pos + pone, pos - none, color);
	__gTraceLine(pos - pone, pos + none, color);
}

inline void __TraceXCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	const auto pone = cliqCity::graphicsMath::Vector3( .5f, .5f, 0);
	const auto none = cliqCity::graphicsMath::Vector3(-.5f, .5f, 0);

	// cross
	__gTraceLine(pos + pone, pos - pone, color);
	__gTraceLine(pos + none, pos - none, color);
}

inline void __TraceCross(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	const auto hone = cliqCity::graphicsMath::Vector3(.5f,   0, 0);
	const auto vone = cliqCity::graphicsMath::Vector3(  0, .5f, 0);

	// cross
	__gTraceLine(pos + vone, pos - vone, color);
	__gTraceLine(pos + hone, pos - hone, color);
}

inline void __TraceDiamond(const cliqCity::graphicsMath::Vector3& pos, const cliqCity::graphicsMath::Vector4& color)
{
	const auto hone = cliqCity::graphicsMath::Vector3(.5f,   0, 0);
	const auto vone = cliqCity::graphicsMath::Vector3(  0, .5f, 0);

	// diamond
	__gTraceLine(pos + vone, pos + hone, color);
	__gTraceLine(pos + hone, pos - vone, color);
	__gTraceLine(pos - vone, pos - hone, color);
	__gTraceLine(pos - hone, pos + vone, color);
}

#define TRACE(message) Trace::GetTrace () << message << Trace::endl
#define TRACE_BOX(position, color) __TraceBox(position, color)
#define TRACE_DIAMOND(position, color) __TraceDiamond(position, color)
#define TRACE_CROSS(position, color) __TraceCross(position, color)
#define TRACE_XCROSS(position, color) __TraceXCross(position, color)
#define TRACE_LINE(from, to, color) __gTraceLine(from, to, color)

#else

#define TRACE(message)
#define TRACE_BOX(position, color)
#define TRACE_DIAMOND(position, color)
#define TRACE_CROSS(position, color)
#define TRACE_XCROSS(position, color)
#define TRACE_LINE(position, color)

#endif