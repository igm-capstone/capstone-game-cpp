#ifndef _WINDLL

#include "Memory.h"
#include <stdio.h>

#define HEAP 1 


int main(int argc, int* argv[])
{

#ifdef HEAP
	uint8_t* buffer[1024];
	LinearAllocator allocator(&buffer[0], &buffer[1024]);
#else
	LinearAllocator allocator(1024);
#endif

	uint8_t* oneByte = (uint8_t*)allocator.Allocate(sizeof(uint8_t), 1, 0);
//	printf("Allocated int8 to address: %p\n", oneByte);

	uint16_t* twoBytes = (uint16_t*)allocator.Allocate(sizeof(uint16_t), 2, 0);
//	printf("Allocated int16 to address: %p\n", twoBytes);

	int32_t* p = (int32_t*)allocator.Allocate(sizeof(int32_t), 4, 0);
//	printf("Allocated int32 to address: %p\n", p);

	int64_t* eightBytes = (int64_t*)allocator.Allocate(sizeof(int64_t), 8, 0);
//	printf("Allocated int64 to address: %p\n", eightBytes);

	allocator.Reset();
	allocator.Free();

//	getchar();

	StackAllocator stackAllocator(buffer, buffer + 1024);

	oneByte = (uint8_t*)stackAllocator.Allocate(sizeof(uint8_t), 1, 0);
	//printf("Allocated int8 to address: %p\n", oneByte);

	twoBytes = (uint16_t*)stackAllocator.Allocate(sizeof(uint16_t), 2, 0);
	//printf("Allocated int16 to address: %p\n", twoBytes);

	p = (int32_t*)stackAllocator.Allocate(sizeof(int32_t), 4, 0);
	//printf("Allocated int32 to address: %p\n", p);

	eightBytes = (int64_t*)stackAllocator.Allocate(sizeof(int64_t), 8, 0);
	//printf("Allocated int64 to address: %p\n", eightBytes);

	stackAllocator.Free(eightBytes);
	stackAllocator.Free(p);
	stackAllocator.Free(twoBytes);
	stackAllocator.Free(oneByte);

	getchar();

	return 0;
}

#endif // !_WINDLL


