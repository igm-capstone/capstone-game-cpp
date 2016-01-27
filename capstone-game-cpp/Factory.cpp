#include "stdafx.h"
#include "Factory.h"
#include "Components\NetworkID.h"
#include "SceneObjects\Explorer.h"

// TODO: decide how to do the memory layout
byte mem[11 * sizeof(NetworkID)];
PoolAllocator FactoryPool<NetworkID>::allocator(mem, mem + 10 * sizeof(NetworkID), sizeof(NetworkID), alignof(NetworkID));

byte mem2[11 * sizeof(NetworkID)];
PoolAllocator FactoryPool<Explorer>::allocator(mem2, mem2 + 10 * sizeof(Explorer), sizeof(Explorer), alignof(NetworkID));
