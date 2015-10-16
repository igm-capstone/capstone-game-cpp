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

	static Trace &Endl(Trace &trace)
	{
		trace.ss << std::endl;
		
		int count = 0;
		while (true)
		{
			//trace.ss.read(trace.buff, 128);
			trace.ss.get(trace.buff, 128);
			count = trace.ss.gcount();

			OutputDebugString(trace.buff);

			if (count < 128)
			{
				break;
			}

		}
		
		trace.ss.clear();
		//trace.ss.seekp(0, std::ios::beg);
		trace.ss.seekg(count-128, std::ios::cur);

		return trace;
	}
};

#define TRACE(message) Trace::GetTrace () << message << Trace::Endl

#else
#define TRACE(message)
#endif