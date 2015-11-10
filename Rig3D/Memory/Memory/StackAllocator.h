#pragma once
#include <stdint.h>

namespace cliqCity
{
	namespace memory
	{
		class StackAllocator
		{
		public:
			StackAllocator(void* start, void* end);
			StackAllocator();
			~StackAllocator();
		
		private:
			uint8_t* mCurrent;
			uint8_t* mStart;
			uint8_t* mEnd;
		};
	}
}

