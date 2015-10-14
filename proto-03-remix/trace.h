#pragma once

#if defined _DEBUG
#include <sstream>
#include <string>
#include <debugapi.h>

class Trace
{
	std::stringstream ss;
public:
	static Trace &GetTrace() { static Trace trace; return trace; }
	Trace &operator << (int value) { ss << value; return *this; }
	Trace &operator << (short value) { ss << value; return *this; }
	Trace &operator << (long value) { ss << value; return *this; }
	Trace &operator << (bool value) { ss << value; return *this; }
	Trace &operator << (char* value) { ss << value; return *this; }
	Trace &operator << (std::string value) { ss << value; return *this; }
	Trace &operator << (Trace &(*function)(Trace &trace)) { return function(*this); }

	static Trace &Endl(Trace &trace)
	{
		trace.ss << std::endl;
		OutputDebugString(trace.ss.str().c_str());
		trace.ss.flush();
		return trace;
	}
};

#define TRACE(message) Trace::GetTrace () << message << Trace::Endl

#else
#define TRACE(message)
#endif