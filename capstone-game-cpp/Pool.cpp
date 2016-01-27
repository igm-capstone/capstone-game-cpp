#include "stdafx.h"
#include "Pool.h"
#include "Components\NetworkID.h"
#include "SceneObjects\Explorer.h"

#pragma region Create Pool Macro

#define CREATE_POOL(type, size)\
static byte __g##type##Buffer[(size) * (sizeof(##type))];\
##type* Pool<##type##>::sBuffer = reinterpret_cast<##type*>(__g##type##Buffer);\
size_t Pool<##type##>::sCount = size;\
PoolAllocator Pool<##type##>::sAllocator(__g##type##Buffer, __g##type##Buffer + ((size) * (sizeof(##type))), sizeof(##type), alignof(##type));

#pragma endregion

// create scene object pools
CREATE_POOL(Explorer, 5)

// create component pools
CREATE_POOL(NetworkID, 10)