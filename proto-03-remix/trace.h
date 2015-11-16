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
	const auto pone = cliqCity::graphicsMath::Vector3(.5f, .5f, 0);
	const auto none = cliqCity::graphicsMath::Vector3(-.5f, .5f, 0);

	// cross
	__gTraceLine(pos + pone, pos - pone, color);
	__gTraceLine(pos + none, pos - none, color);

	// box
	__gTraceLine(pos + pone, pos + none, color);
	__gTraceLine(pos - pone, pos - none, color);
	__gTraceLine(pos + pone, pos - none, color);
	__gTraceLine(pos - pone, pos + none, color);
}

#define TRACE(message) Trace::GetTrace () << message << Trace::endl
#define TRACE_BOX(position, color) __TraceBox(position, color)
#define TRACE_LINE(from, to, color) __gTraceLine(from, to color)

#else
#define TRACE(message)
#define TRACE_BOX(position, color)
#define TRACE_LINE(position, color)
#endif