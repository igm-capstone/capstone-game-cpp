#include "stdafx.h"
#include "Resource.h"
#include "json.h"
#include "SceneObjects/Wall.h"
#include "SceneObjects/MoveableBlock.h"
#include "SceneObjects/SpawnPoint.h"
#include "SceneObjects/Pickup.h"
#include "SceneObjects/DominationPoint.h"
#include "trace.h"
#include "SceneObjects/Lamp.h"
#include <GraphicsMath/cgm.h>

using namespace std;
using namespace Rig3D;
using namespace nlohmann;

using jstr_t = string;
using jarr_t = vector<json>*;
using jobj_t = map<jstr_t, json>*;


inline vec3f parseVec3f(json js)
{
	return vec3f(js[0].get<float>(), js[1].get<float>(), js[2].get<float>());
}


inline quatf parseQuatf(json js)
{
	return quatf(js[3].get<float>(), js[0].get<float>(), js[1].get<float>(), js[2].get<float>());
}


void parseTransform(json obj, Transform* transform)
{

	auto position = obj["position"];
	if (!position.empty()) 
	{
		transform->SetPosition(parseVec3f(position));
	}

	auto rotation = obj["rotation"];
	if (!rotation.empty())
	{
		transform->SetRotation(parseQuatf(rotation));
	}

	auto scale = obj["scale"];
	if (!scale.empty())
	{
		transform->SetScale(parseVec3f(scale));
	}
}


void loadLamps(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " lamps...");
	for (auto obj : *objs)
	{
		auto lamp = Factory<Lamp>::Create();
		parseTransform(obj, lamp->mTransform);

		lamp->mLightRadius = obj["lightRadius"].get<float>();
	}
}


void loadDomPoints(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " domination points...");
	for (auto obj : *objs)
	{
		auto dom = Factory<DominationPoint>::Create();
		parseTransform(obj, dom->mTransform);

		dom->mDominationTime = obj["captureTime"].get<float>();
		dom->mTier = obj["tier"].get<int>();
	}
}


void loadPickups(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " pickups...");
	for (auto obj : *objs)
	{
		auto pickup = Factory<Pickup>::Create();
		parseTransform(obj, pickup->mTransform);

		pickup->mSkill = obj["skillName"].get<string>();
	}
}


void loadSpawnPoints(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " spawn points...");
	for (auto obj : *objs)
	{
		auto spawnPoint = Factory<SpawnPoint>::Create();
		parseTransform(obj, spawnPoint->mTransform);
	}
}


void loadWalls(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " walls...");
	for (auto obj : *objs)
	{
		auto wall = Factory<Wall>::Create();
		parseTransform(obj, wall->mTransform);
		wall->mBoxCollider->mCollider.origin = wall->mTransform->GetPosition();
		wall->mBoxCollider->mCollider.halfSize = wall->mTransform->GetScale() * 0.5f;
		
		mat3f axis = wall->mTransform->GetRotationMatrix();
		wall->mBoxCollider->mCollider.axis[0] = axis.pRows[0];
		wall->mBoxCollider->mCollider.axis[1] = axis.pRows[1];
		wall->mBoxCollider->mCollider.axis[2] = axis.pRows[2];
	}
}


void loadBlocks(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " moveable blocks...");
	for (auto obj : *objs)
	{
		auto position = parseVec3f(obj["position"]);
		auto rotation = parseQuatf(obj["rotation"]);
		auto scale = parseVec3f(obj["scale"]);

		auto block = Factory<MoveableBlock>::Create();
		parseTransform(obj, block->mTransform);

		block->mTransform->SetPosition(position);
		block->mTransform->SetRotation(rotation);
		block->mTransform->SetScale(scale);
	}
}


Resource::LevelInfo Resource::LoadLevel(string path, LinearAllocator& allocator)
{
	auto fstream = ifstream(path);
	auto obj = json::parse(fstream);
	fstream.close();

	LevelInfo level;
	memset(&level, 0, sizeof(level));
	
	auto center = obj["metadata"]["bounds"]["center"];
	if (center != nullptr)
	{
		level.center = parseVec3f(center);
	}

	auto extents = obj["metadata"]["bounds"]["extents"];
	if (extents != nullptr)
	{
		level.extents = parseVec3f(extents);
	}

	level.tileCount = TILE_COUNT_X * TILE_COUNT_Y;
	level.tiles = reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * level.tileCount, alignof(mat4f), 0));

	float levelWidth	= level.extents.x * 2.0f;
	float levelHeight	= level.extents.x * 2.0f;

	level.tileWidth		=	levelWidth / static_cast<float>(TILE_COUNT_X);
	level.tileHeight	=	levelHeight / static_cast<float>(TILE_COUNT_Y);
	
	float halfWidth		= (levelWidth - level.tileWidth) * 0.5f;
	float halfHeight	= (levelHeight - level.tileHeight) * 0.5f;

	for (int y = 0; y < TILE_COUNT_Y; y++)
	{
		for (int x = 0; x < TILE_COUNT_X; x++)
		{
			level.tiles[y * TILE_COUNT_X + x] = (mat4f::rotateX(-PI * 0.5f) * mat4f::translate({ x * level.tileWidth - halfWidth, halfHeight - y * level.tileHeight, 0.0f })).transpose();
		}
	}

	auto lamps = obj["lamp"].get_ptr<jarr_t>();
	if (lamps != nullptr)
	{
		loadLamps(lamps);
	}

	auto domination = obj["domination"].get_ptr<jarr_t>();
	if (domination != nullptr)
	{
		loadDomPoints(domination);
	}

	auto pickups = obj["pickup"].get_ptr<jarr_t>();
	if (pickups != nullptr)
	{
		loadPickups(pickups);
	}

	auto spawnPoints = obj["spawnPoint"].get_ptr<jarr_t>();
	if (spawnPoints != nullptr)
	{
		loadSpawnPoints(spawnPoints);
	}

	auto moveableBlocks = obj["moveableBlock"].get_ptr<jarr_t>();
	if (moveableBlocks != nullptr)
	{
		loadBlocks(moveableBlocks);
	}

	auto walls = obj["wall"].get_ptr<jarr_t>();
	if (walls != nullptr)
	{
		loadWalls(walls);
	}

	level.wallCount = static_cast<short>(walls->size());
	level.walls = reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * level.wallCount, alignof(mat4f), 0));

	int i = 0;
	for (Wall& w : Factory<Wall>())
	{
		level.walls[i++] = w.mTransform->GetWorldMatrix().transpose();
	}

	return level;
}

