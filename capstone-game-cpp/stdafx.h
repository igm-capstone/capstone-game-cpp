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

//Config

#define MAX_EXPLORERS 4
#define MAX_EXPLORER_SKILLS  5

#define MAX_GHOST_SKILLS 4

#define MAX_MINIONS 50

#define MAX_STATIC_MESHES 1500

#define MAX_STATIC_COLLIDERS 300

#define SHADOW_MAP_SIZE 1024

#define MAX_COLLISIONS 128

#define MAX_SPRITES (MAX_EXPLORERS+MAX_MINIONS)

#define MAX_SKELETON_JOINTS 64

#define GRID_MULT_OF	12