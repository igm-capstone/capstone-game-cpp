#pragma once
#ifndef ALLOCATOR_UTILITY
#define ALLOCATOR_UTILITY

#define EXTERN_C(function) extern "C" { function }

#ifdef __cplusplus
EXTERN_C(void* AlignedPointer(void* buffer, unsigned int alignment););
#else
void* AlignedPointer(void* buffer, unsigned int alignment);
#endif // cplusplus


#endif // ALLOCATOR_UTILITY