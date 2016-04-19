#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <d3d11.h>
#include "Rig3D\Engine.h"

#include "Memory\Memory\LinearAllocator.h"

#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Graphics\MeshLibrary.h"

#include "Rig3D\Common\Transform.h"

#include "Network\NetworkManager.h"

//Debug

extern bool gDebugExplorer;
extern bool gDebugGrid;
extern bool gDebugOrto;
extern bool gDebugColl;
extern bool gDebugGBuffer;
extern bool gDebugBVH;
extern bool gDebugBT;

//Config

#define STATIC_APP_MEMORY		8000000
#define STATIC_SCENE_MEMORY		2000000
#define SCENE_ALIGNMENT_PADDING 6

#define MAX_EXPLORERS 4
#define MAX_EXPLORER_SKILLS  5

#define MAX_GHOST_SKILLS 5

#define GHOST_MANA_MAX 100
#define GHOST_MANA_REGEN 5


#define MAX_IMPS          30
#define MAX_ABOMINATIONS  10
#define MAX_FLYTRAPS      10
#define MAX_MINIONS       MAX_IMPS + MAX_ABOMINATIONS + MAX_FLYTRAPS

#define MAX_STATIC_MESHES 1800

#define MAX_STATIC_COLLIDERS 300

#define SHADOW_MAP_SIZE 1024
#define GRID_MAP_SCALE	10

#define MAX_COLLISIONS 128

#define MAX_GLYPHS 10000
#define MAX_SPRITES (MAX_EXPLORERS+MAX_MINIONS)
#define MAX_SPRITESHEETS 10

#define MAX_SKELETON_JOINTS 64

#define GRID_MULT_OF	6

#define FIXED_UPDATE 60.0  // X fixed updates per sec

#define MAX_DOORS 50
#define MAX_LAMPS 50

#define DOOR_BASE_UUID 0xDD000000
#define LAMP_BASE_UUID 0xAA000000
