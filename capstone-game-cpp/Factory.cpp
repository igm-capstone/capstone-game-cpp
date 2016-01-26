#include "stdafx.h"
#include "Factory.h"

// TODO: decide how to do the memory layout
byte mem[1090];

PoolAllocator ComponentPool<CompA>::allocator = PoolAllocator(mem, mem + 10 * sizeof(CompA), sizeof(CompA));
