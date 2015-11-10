#pragma once

#ifdef _WINDLL
#define MEMORY_API __declspec(dllexport)
#else
#define MEMORY_API __declspec(dllimport)
#endif

namespace cliqCity
{
	namespace memory
	{
		class MEMORY_API PoolAllocator
		{
		public:
			typedef PoolAllocator Node;

			PoolAllocator(void* start, void* end, size_t elementSize);
			void* Allocate();
			void  Free(void* pointer);

		private:
			Node* mNext;
		};
	}
}