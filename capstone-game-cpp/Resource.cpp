#include "stdafx.h"
#include "Resource.h"
#include "json.h"
#include "SceneObjects/Wall.h"
#include "SceneObjects/MoveableBlock.h"
#include "SceneObjects/SpawnPoint.h"
#include "SceneObjects/Pickup.h"
#include "SceneObjects/DominationPoint.h"
#include "trace.h"

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


void loadDomPoints(jarr_t objs)
{
	TRACE("Loading " << int(objs->size()) << " domination points...");
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
	TRACE("Loading " << int(objs->size()) << " pickups...");
	for (auto obj : *objs)
	{
		auto pickup = Factory<Pickup>::Create();
		parseTransform(obj, pickup->mTransform);

		pickup->mSkill = obj["skillName"].get<string>();
	}
}


void loadSpawnPoints(jarr_t objs)
{
	TRACE("Loading " << int(objs->size()) << " spawn points...");
	for (auto obj : *objs)
	{
		auto spawnPoint = Factory<SpawnPoint>::Create();
		parseTransform(obj, spawnPoint->mTransform);
	}
}


void loadWalls(jarr_t objs)
{
	TRACE("Loading " << int(objs->size()) << " walls...");
	for (auto obj : *objs)
	{
		auto wall = Factory<Wall>::Create();
		parseTransform(obj, wall->mTransform);
	}
}


void loadBlocks(jarr_t objs)
{
	TRACE("Loading " << int(objs->size()) << " moveable blocks...");
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
	
	auto light = obj["light"].get_ptr<jarr_t>();
	if (light != nullptr)
	{
		//level.lights = loadTransforms(light, allocator);
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

	return level;
}

