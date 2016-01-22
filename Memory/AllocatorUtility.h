#pragma once
#ifndef ALLOCATOR_UTILITY
#define ALLOCATOR_UTILITY

#define EXTERN_C(function) extern "C" { function }

#ifdef __cplusplus
EXTERN_C(void*	AlignedPointer(void* buffer, unsigned int alignment););
EXTERN_C(void*	RawPointer(void* alignedPointer););
EXTERN_C(char	Adjustment(void* alignedPointer););

#else
void* AlignedPointer(void* buffer, unsigned int alignment);
void* RawPointer(void* alignedPointer);
char  Adjustment(void* alignedPointer);
#endif // cplusplus


#endif // ALLOCATOR_UTILITY