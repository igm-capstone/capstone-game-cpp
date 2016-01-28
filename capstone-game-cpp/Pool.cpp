#include "stdafx.h"
#include "Pool.h"
#include "Components\NetworkID.h"
#include "SceneObjects\Explorer.h"
#include "SceneObjects\Wall.h"
#include "SceneObjects\MoveableBlock.h"
#include "SceneObjects\SpawnPoint.h"
#include "SceneObjects\Pickup.h"

#pragma region Create Pool Macro

#define CREATE_POOL(type, size)\
static byte __g##type##Buffer[(size) * (sizeof(##type))];\
##type* Factory<##type##>::sBuffer = reinterpret_cast<##type*>(__g##type##Buffer);\
size_t Factory<##type##>::sCount = size;\
PoolAllocator Factory<##type##>::sAllocator(__g##type##Buffer, __g##type##Buffer + ((size) * (sizeof(##type))), sizeof(##type), alignof(##type));

#pragma endregion

// create scene object Factorys
CREATE_POOL ( Explorer,			5	)
CREATE_POOL ( Wall,				100	)
CREATE_POOL ( MoveableBlock,	10	)
CREATE_POOL ( SpawnPoint,		5	)
CREATE_POOL ( Pickup,			5	)

CREATE_POOL ( Transform,		105 )

// create component pools
CREATE_POOL ( NetworkID,			10	)
CREATE_POOL ( BoxColliderComponent,	100	)

