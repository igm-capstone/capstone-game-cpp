#include "stdafx.h"
#include "Factory.h"

#include "SceneObjects\Minion.h"
#include "SceneObjects\FlyTrap.h"
#include "SceneObjects\StaticCollider.h"
#include "SceneObjects\StaticMesh.h"
#include "SceneObjects\MoveableBlock.h"
#include "SceneObjects\SpawnPoint.h"
#include "SceneObjects\Pickup.h"
#include "SceneObjects\DominationPoint.h"
#include "SceneObjects\Lamp.h"
#include "SceneObjects\Ghost.h"
#include "SceneObjects\Region.h"
#include "SceneObjects\Door.h"

#include "Components\NetworkID.h"
#include "Components\ColliderComponent.h"
#include "Components\ExplorerController.h"
#include "Components\GhostController.h"
#include "Components\FlyTrapController.h"
#include "Components\MinionController.h"
#include "Components\DominationPointController.h"
#include "Components\Skill.h"
#include "Components\Health.h"
#include "Components\FmodEvent.h"
#include "Components\FmodEventCollection.h"
#include "Components\AnimationController.h"

#pragma region REGISTER_FACTORY Macro

#define REGISTER_FACTORY(type, size)\
static byte __g##type##Buffer[(size) * (sizeof(##type)) + alignof(##type)];\
##type* Factory<##type##>::sBuffer = reinterpret_cast<##type*>(__g##type##Buffer);\
size_t Factory<##type##>::sMaxCount = size;\
size_t Factory<##type##>::sCount = 0;\
PoolAllocator Factory<##type##>::sAllocator(__g##type##Buffer, __g##type##Buffer + (size) * (sizeof(##type)) + alignof(##type), sizeof(##type), alignof(##type));

#pragma endregion

// register scene object factories
REGISTER_FACTORY ( Ghost,			1	)
REGISTER_FACTORY ( Explorer,		MAX_EXPLORERS)
REGISTER_FACTORY ( Minion,			MAX_MINIONS	)
REGISTER_FACTORY ( FlyTrap,			MAX_MINIONS)
REGISTER_FACTORY ( StaticMesh,		MAX_STATIC_MESHES)
REGISTER_FACTORY ( StaticCollider,	MAX_STATIC_COLLIDERS	)
REGISTER_FACTORY ( MoveableBlock,	10	)
REGISTER_FACTORY ( SpawnPoint,		5	)
REGISTER_FACTORY ( Pickup,			7	)
REGISTER_FACTORY ( DominationPoint,	6	)
REGISTER_FACTORY ( Lamp,			33	)
REGISTER_FACTORY ( Region,			100)


// register transform factory
REGISTER_FACTORY ( Transform,	100 + MAX_STATIC_MESHES + MAX_EXPLORERS + MAX_MINIONS + MAX_STATIC_COLLIDERS)

// register component factories
REGISTER_FACTORY ( NetworkID,						1 + MAX_EXPLORERS + MAX_MINIONS)
REGISTER_FACTORY ( BoxColliderComponent,			100 + MAX_STATIC_MESHES)
REGISTER_FACTORY ( OrientedBoxColliderComponent,	MAX_STATIC_COLLIDERS + 2*MAX_DOORS)
REGISTER_FACTORY ( SphereColliderComponent,			MAX_EXPLORERS + MAX_MINIONS + MAX_LAMPS)
REGISTER_FACTORY ( ExplorerController,				MAX_EXPLORERS)
REGISTER_FACTORY ( GhostController,					1	)
REGISTER_FACTORY ( MinionController,				MAX_MINIONS)
REGISTER_FACTORY ( FlyTrapController,				MAX_MINIONS)

REGISTER_FACTORY ( DominationPointController,		5	)
REGISTER_FACTORY ( Skill,							20	)
REGISTER_FACTORY ( Health,							MAX_EXPLORERS + MAX_MINIONS)
REGISTER_FACTORY ( FmodEvent,						100	)
REGISTER_FACTORY ( FmodEventCollection,				10	)
REGISTER_FACTORY ( AnimationController,				MAX_EXPLORERS + MAX_MINIONS)
REGISTER_FACTORY ( PlaneColliderComponent,			100)
REGISTER_FACTORY ( Door,							MAX_DOORS)