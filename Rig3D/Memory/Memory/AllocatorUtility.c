#include "AllocatorUtility.h"
#include <assert.h>

void* AlignedPointer(void* buffer, unsigned int alignment)
{
	// Create mask to round down.
	char mask = (alignment - 1);
	char* rawPointer = (char*)buffer;

	char misalignment = (char)rawPointer & mask;
	char adjustment = alignment - misalignment;
	char* alignedPointer = rawPointer + adjustment;

	assert(adjustment < 256, __FILE__, __LINE__);
	alignedPointer[-1] = adjustment;

	return (void*)alignedPointer;
}
	
