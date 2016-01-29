#include "stdafx.h"
#include "Factory.h"
#include "Components\NetworkID.h"
#include "SceneObjects\Explorer.h"
#include "SceneObjects\Wall.h"
#include "SceneObjects\MoveableBlock.h"
#include "SceneObjects\SpawnPoint.h"
#include "SceneObjects\Pickup.h"
#include "SceneObjects\DominationPoint.h"

#pragma region REGISTER_FACTORY Macro

#define REGISTER_FACTORY(type, size)\
static byte __g##type##Buffer[(size) * (sizeof(##type))];\
##type* Factory<##type##>::sBuffer = reinterpret_cast<##type*>(__g##type##Buffer);\
size_t Factory<##type##>::sCount = size;\
PoolAllocator Factory<##type##>::sAllocator(__g##type##Buffer, __g##type##Buffer + ((size) * (sizeof(##type))), sizeof(##type), alignof(##type));

#pragma endregion

// register scene object factories
REGISTER_FACTORY ( Explorer,		5	)
REGISTER_FACTORY ( Wall,			100	)
REGISTER_FACTORY ( MoveableBlock,	10	)
REGISTER_FACTORY ( SpawnPoint,		5	)
REGISTER_FACTORY ( Pickup,			7	)
REGISTER_FACTORY ( DominationPoint,	6	)

// register transform factory
REGISTER_FACTORY ( Transform,	105 )

// register component factories
REGISTER_FACTORY ( NetworkID,				10	)
REGISTER_FACTORY ( BoxColliderComponent,	100	)
REGISTER_FACTORY ( SphereColliderComponent, 100 )
REGISTER_FACTORY ( ExplorerController,		5	)