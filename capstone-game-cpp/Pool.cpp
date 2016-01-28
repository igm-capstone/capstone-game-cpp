#include "stdafx.h"
#include "Pool.h"
#include "Components\NetworkID.h"
#include "SceneObjects\Explorer.h"

#pragma region Create Pool Macro

#define CREATE_POOL(type, size)\
static byte __g##type##Buffer[(size) * (sizeof(##type))];\
##type* Factory<##type##>::sBuffer = reinterpret_cast<##type*>(__g##type##Buffer);\
size_t Factory<##type##>::sCount = size;\
PoolAllocator Factory<##type##>::sAllocator(__g##type##Buffer, __g##type##Buffer + ((size) * (sizeof(##type))), sizeof(##type), alignof(##type));

#pragma endregion

// create scene object Factorys
CREATE_POOL(Explorer, 5)

// create component pools
CREATE_POOL(NetworkID, 10)