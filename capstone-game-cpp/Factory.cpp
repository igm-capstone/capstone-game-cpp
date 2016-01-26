#include "stdafx.h"
#include "Factory.h"
#include "Components\NetworkID.h"
#include "SceneObjects\Explorer.h"

// TODO: decide how to do the memory layout
byte mem[11 * sizeof(NetworkID)];
PoolAllocator FactoryPool<NetworkID>::allocator = PoolAllocator(mem, mem + 10 * sizeof(NetworkID), sizeof(NetworkID));

byte mem2[11 * sizeof(NetworkID)];
PoolAllocator FactoryPool<Explorer>::allocator = PoolAllocator(mem2, mem2 + 10 * sizeof(Explorer), sizeof(Explorer));
