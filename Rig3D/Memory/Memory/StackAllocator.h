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
		
			void*	Allocate(size_t size, size_t alignment, size_t offset);
			void	Free(void* ptr);

		private:
			uint8_t* mCurrent;
			uint8_t* mStart;
			uint8_t* mEnd;
		};
	}
}

