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

#define MAX_PLAYERS 4
#define MAX_EXPLORERS 3
#define MAX_EXPLORER_SKILLS  5

#define MAX_GHOST_SKILLS 5

#define MAX_IMPS          30
#define MAX_ABOMINATIONS  30
#define MAX_FLYTRAPS      30
#define MAX_MINIONS       100

#define MAX_STATIC_MESHES 2500

#define MAX_STATIC_COLLIDERS 300

#define SHADOW_MAP_SIZE 1024
#define GRID_MAP_SCALE	10

#define MAX_COLLISIONS 128

#define MAX_GLYPHS 10000
#define MAX_SPRITES (100 + MAX_EXPLORERS+MAX_MINIONS)
#define MAX_SPRITESHEETS 16
#define MAX_INTERACTIBLE_AREAS 20

#define MAX_SKELETON_JOINTS 64

#define GRID_MULT_OF	6

#define FIXED_UPDATE 100.0  // X fixed updates per sec, also fixed FPS.

#define MAX_DOORS 50
#define MAX_LAMPS 50

#define DOOR_BASE_UUID 0xDD000000
#define LAMP_BASE_UUID 0xAA000000
