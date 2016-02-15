#include "stdafx.h"
#include "Factory.h"
#include "SceneObjects\Explorer.h"
#include "SceneObjects\Minion.h"
#include "SceneObjects\Wall.h"
#include "SceneObjects\MoveableBlock.h"
#include "SceneObjects\SpawnPoint.h"
#include "SceneObjects\Pickup.h"
#include "SceneObjects\DominationPoint.h"
#include "SceneObjects\Lamp.h"
#include "SceneObjects\Ghost.h"

#pragma region REGISTER_FACTORY Macro

#define REGISTER_FACTORY(type, size)\
static byte __g##type##Buffer[(size) * (sizeof(##type)) + alignof(##type)];\
##type* Factory<##type##>::sBuffer = reinterpret_cast<##type*>(__g##type##Buffer);\
size_t Factory<##type##>::sCount = size;\
PoolAllocator Factory<##type##>::sAllocator(__g##type##Buffer, __g##type##Buffer + (size) * (sizeof(##type)) + alignof(##type), sizeof(##type), alignof(##type));

#pragma endregion

// register scene object factories
REGISTER_FACTORY ( Ghost,			1	)
REGISTER_FACTORY ( Explorer,		MAX_EXPLORERS)
REGISTER_FACTORY ( Minion,			MAX_MINIONS	)
REGISTER_FACTORY ( Wall,			100	)
REGISTER_FACTORY ( MoveableBlock,	10	)
REGISTER_FACTORY ( SpawnPoint,		5	)
REGISTER_FACTORY ( Pickup,			7	)
REGISTER_FACTORY ( DominationPoint,	6	)
REGISTER_FACTORY ( Lamp,			17	)

// register transform factory
REGISTER_FACTORY ( Transform,	200 )

// register component factories
REGISTER_FACTORY ( NetworkID,						MAX_EXPLORERS	)
REGISTER_FACTORY ( BoxColliderComponent,			100	)
REGISTER_FACTORY ( OrientedBoxColliderComponent,	100 )
REGISTER_FACTORY ( SphereColliderComponent,			100 )
REGISTER_FACTORY ( ExplorerController,				MAX_EXPLORERS)
REGISTER_FACTORY ( GhostController,					1	)
REGISTER_FACTORY ( MinionController,				MAX_MINIONS)
REGISTER_FACTORY ( Skill,							20	)
REGISTER_FACTORY ( Health,							MAX_EXPLORERS + MAX_MINIONS)
REGISTER_FACTORY ( FmodEvent,						100	)
REGISTER_FACTORY ( FmodEventCollection,				10	)
REGISTER_FACTORY ( AnimationController,				MAX_EXPLORERS + MAX_MINIONS)