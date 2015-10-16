#pragma once

#if defined _DEBUG
#include <sstream>
#include <string>
#include <debugapi.h>

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

			OutputDebugString(str.c_str());
			OutputDebugString("\n");
		}

		trace.ss.clear();

		return trace;
	}
};

#define TRACE(message) Trace::GetTrace () << message << Trace::endl

#else
#define TRACE(message)
#endif