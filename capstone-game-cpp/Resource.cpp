#include "stdafx.h"
#include "Resource.h"
#include "json.h"

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


Transform* loadTransforms(jarr_t objs, LinearAllocator& allocator)
{
	auto size = sizeof(Transform) * objs->size();
	auto transforms = reinterpret_cast<Transform*>
		(allocator.Allocate(size, alignof(Transform), 0));
	memset(transforms, 0, size);

	auto transform = transforms;
	for (auto obj : *objs)
	{
		auto position = parseVec3f(obj["position"]);
		auto rotation = parseQuatf(obj["rotation"]);
		auto scale = parseVec3f(obj["scale"]);
		
		transform->SetPosition(position);
		transform->SetRotation(rotation);
		transform->SetScale(scale);

		transform++;
	}

	return transforms;
}

mat4f* loadMatrices(jarr_t objs, LinearAllocator& allocator)
{
	auto transforms = reinterpret_cast<mat4f*>
		(allocator.Allocate(sizeof(mat4f) * objs->size(), alignof(mat4f), 0));

	auto transform = transforms;
	for (auto obj : *objs)
	{
		mat4f translation = mat4f::translate(parseVec3f(obj["position"]));
		mat4f rotation = parseQuatf(obj["rotation"]).toMatrix4();
		mat4f scale = mat4f::scale(parseVec3f(obj["scale"]));

		*transform = scale * rotation * translation;

		transform++;
	}

	return transforms;
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
		
	}

	auto pickup = obj["pickup"].get_ptr<jarr_t>();
	if (pickup != nullptr)
	{
		
	}

	auto spawnPoint = obj["spawnPoint"].get_ptr<jarr_t>();
	if (spawnPoint != nullptr)
	{
		
	}

	auto moveableBlocks = obj["moveableBlock"].get_ptr<jarr_t>();
	if (moveableBlocks != nullptr)
	{
		level.moveableBlocks = loadTransforms(moveableBlocks, allocator);
	}

	auto walls = obj["wall"].get_ptr<jarr_t>();
	if (walls != nullptr)
	{
		level.walls = loadMatrices(walls, allocator);
		level.wallCount = walls->size();
	}

	return level;
}

