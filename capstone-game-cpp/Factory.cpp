#include "stdafx.h"
#include "Factory.h"

#include "SceneObjects\Minion.h"
#include "SceneObjects\StaticCollider.h"
#include "SceneObjects\StaticMesh.h"
#include "SceneObjects\SpawnPoint.h"
#include "SceneObjects\DominationPoint.h"
#include "SceneObjects\Lamp.h"
#include "SceneObjects\Ghost.h"
#include "SceneObjects\Region.h"
#include "SceneObjects\Door.h"
#include "SceneObjects\Heal.h"
#include "SceneObjects\Explosion.h"
#include "SceneObjects\Trap.h"
#include "SceneObjects\StatusEffect.h"
#include "SceneObjects\Lantern.h"
#include "SceneObjects\Transmogrify.h"

#include "Components\NetworkID.h"
#include "Components\ColliderComponent.h"
#include "Components\ExplorerController.h"
#include "Components\GhostController.h"
#include "Components\FlyTrapController.h"
#include "Components\AbominationController.h"
#include "Components\ImpController.h"
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

// register transform factory
REGISTER_FACTORY ( Transform,	100 + MAX_STATIC_MESHES + MAX_EXPLORERS + MAX_MINIONS + MAX_STATIC_COLLIDERS)

// register SCENE OBJECTS factories
REGISTER_FACTORY ( Ghost,			1	)
REGISTER_FACTORY ( Explorer,		MAX_EXPLORERS)
REGISTER_FACTORY ( Minion,			MAX_MINIONS	)
REGISTER_FACTORY ( StaticMesh,		MAX_STATIC_MESHES)
REGISTER_FACTORY ( StaticCollider,	MAX_STATIC_COLLIDERS)
REGISTER_FACTORY ( SpawnPoint,		5	)
REGISTER_FACTORY ( DominationPoint,	6	)
REGISTER_FACTORY ( Lamp,			MAX_LAMPS)
REGISTER_FACTORY ( Region,			100)
REGISTER_FACTORY ( Door,			MAX_DOORS)

// register component factories
REGISTER_FACTORY ( NetworkID,						1 + MAX_EXPLORERS + MAX_MINIONS + MAX_LAMPS + MAX_DOORS)
REGISTER_FACTORY ( BoxColliderComponent,			100 )
REGISTER_FACTORY ( OrientedBoxColliderComponent,	MAX_STATIC_COLLIDERS + MAX_STATIC_MESHES + 2*MAX_DOORS)
REGISTER_FACTORY ( SphereColliderComponent,			MAX_EXPLORERS + MAX_MINIONS + MAX_LAMPS)
REGISTER_FACTORY ( ExplorerController,				MAX_EXPLORERS)
REGISTER_FACTORY ( GhostController,					1	)
REGISTER_FACTORY ( ImpController,					MAX_IMPS)
REGISTER_FACTORY ( AbominationController,			MAX_ABOMINATIONS)
REGISTER_FACTORY ( FlyTrapController,				MAX_FLYTRAPS)

REGISTER_FACTORY ( DominationPointController,		5	)
REGISTER_FACTORY ( Skill,							MAX_EXPLORERS * MAX_EXPLORER_SKILLS + MAX_GHOST_SKILLS)
REGISTER_FACTORY ( Health,							MAX_EXPLORERS + MAX_MINIONS)
REGISTER_FACTORY ( FmodEvent,						100	)
REGISTER_FACTORY ( FmodEventCollection,				10	)
REGISTER_FACTORY ( AnimationController,				MAX_EXPLORERS + MAX_MINIONS)
REGISTER_FACTORY ( PlaneColliderComponent,			100)

REGISTER_FACTORY ( Heal, 5)
REGISTER_FACTORY ( Explosion, 10)
REGISTER_FACTORY ( Trap, 5)
REGISTER_FACTORY ( StatusEffect, 8)
REGISTER_FACTORY ( Lantern, 10)
REGISTER_FACTORY ( Transmogrify, 3)
