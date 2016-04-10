#include "stdafx.h"
#include "Resource.h"
#include "json.h"
#include "SceneObjects/MoveableBlock.h"
#include "SceneObjects/SpawnPoint.h"
#include "SceneObjects/Pickup.h"
#include "SceneObjects/DominationPoint.h"
#include "trace.h"
#include "SceneObjects/Lamp.h"
#include <GraphicsMath/cgm.h>
#include "SceneObjects/StaticMesh.h"
#include "SceneObjects/StaticCollider.h"
#include "SceneObjects/Region.h"
#include "SceneObjects/Door.h"

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


void parseStaticMeshTransform(json obj, Transform* transform)
{

	auto position = obj["position"];
	if (!position.empty())
	{
		transform->SetPosition(parseVec3f(position));
	}

	auto rotation = obj["rotation"];
	if (!rotation.empty())
	{
		// In Unity, static meshes have a child elemented with rotation (-90,0,0). This is not part of the JSON file.
		transform->SetRotation(parseQuatf(rotation)*quatf::rollPitchYaw(0, -0.5*PI, 0));
	} 
	else 
	{
		transform->SetRotation(quatf::rollPitchYaw(0, -0.5*PI, 0));
	}

	auto scale = obj["scale"];
	if (!scale.empty())
	{
		// Again, because of the the child rotation, the z<->y scale is flipped;
		auto json = parseVec3f(scale);
		transform->SetScale(vec3f(json.x, json.z, json.y));
	}
}

void loadLamps(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " lamps...");
	
	vec3f defaultDirection = { 1.0f, 0.0f, 0.0f };

	float radians = PI / 180.0f;

	for (auto obj : *objs)
	{
		auto lamp = Factory<Lamp>::Create();
		parseTransform(obj, lamp->mTransform);

		lamp->mLightColor		= { 1.0f, 1.0f, 1.0f, 1.0f };
		lamp->mLightDirection	= defaultDirection * lamp->mTransform->GetRotationMatrix();
		lamp->mLightRadius		= obj["lightRadius"].get<float>();
		lamp->mLightAngle		= obj["angle"].get<float>() * radians;
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
		dom->mCollider->mCollider.origin = dom->mTransform->GetPosition();
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


void loadStaticMeshes(jarr_t objs, std::string model, vector<string>& textureNames, vector<uint32_t>& materialIDs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " static meshes...");

	for (auto obj : *objs)
	{
		auto staticMesh = Factory<StaticMesh>::Create();
		parseStaticMeshTransform(obj, staticMesh->mTransform);
		Resource::mModelManager->GetModel(model.c_str())->Link(staticMesh);

		auto textureName = obj["textureName"];
		if (!textureName.empty())
		{
			string str = textureName.get<string>();
			if (str.length() == 0)
			{
				str = "debugTexture.png";
			}

			vector<string>::iterator iter = find(textureNames.begin(), textureNames.end(), str);
			if (iter != textureNames.end())
			{
				materialIDs.push_back(iter - textureNames.begin());
			}
			else
			{
				textureNames.push_back(str);
				materialIDs.push_back(textureNames.size() - 1);
			}
		}

		auto bounds = obj["bounds"];
		if (!bounds.empty())
		{
			staticMesh->mColliderComponent->mLayer = COLLISION_LAYER_WALL;

			auto center = bounds["center"];
			if (!center.empty())
			{
				staticMesh->mColliderComponent->mCollider.origin = parseVec3f(center);
			}

			auto extents = bounds["extents"];
			if (!extents.empty())
			{
				staticMesh->mColliderComponent->mCollider.halfSize = parseVec3f(extents);
			}
		}

	}
}

void loadRegions(jarr_t objs, CLayer layer, vec3f levelOrigin, vec3f levelExtents)
{
	TRACE_LOG("Loading " << int(objs->size()) << " static regions...");
	for (auto obj : *objs)
	{
		auto region = Factory<Region>::Create();
		parseTransform(obj, region->mTransform);
		
		vec3f adjustedScale = region->mTransform->GetScale();
		adjustedScale.z = 1.0f;
		region->mTransform->SetScale(adjustedScale);

		// Bounding volume construction for BVH Tree
		region->mColliderComponent->mCollider.origin = region->mTransform->GetPosition();
		region->mColliderComponent->mCollider.halfSize = region->mTransform->GetScale() * 0.5f;

		mat3f axis = region->mTransform->GetRotationMatrix();
		region->mColliderComponent->mCollider.axis[0] = axis.pRows[0];
		region->mColliderComponent->mCollider.axis[1] = axis.pRows[1];
		region->mColliderComponent->mCollider.axis[2] = axis.pRows[2];
	}
}

void loadStaticColliders(jarr_t objs, CLayer layer, vec3f levelOrigin, vec3f levelExtents)
{
	TRACE_LOG("Loading " << int(objs->size()) << " static colliders...");
	for (auto obj : *objs)
	{
		auto collider = Factory<StaticCollider>::Create();
		parseTransform(obj, collider->mTransform);

		// Hard coding z scale and position for colliders based on z extents in Unity
		// Min z = 0.09 Max z = 15.06
		// Lucas please give me z extents in json when you have time :)
		vec3f jsonPosition	= collider->mTransform->GetPosition();
		collider->mTransform->SetPosition(jsonPosition.x, jsonPosition.y,-7.5f);

		vec3f jsonScale = collider->mTransform->GetScale();
		collider->mTransform->SetScale(jsonScale.x, jsonScale.y, 15.0f);

		collider->mColliderComponent->mCollider.origin = collider->mTransform->GetPosition();
		collider->mColliderComponent->mCollider.halfSize = collider->mTransform->GetScale() * 0.5f;

		mat3f axis = collider->mTransform->GetRotationMatrix();
		collider->mColliderComponent->mCollider.axis[0] = axis.pRows[0];
		collider->mColliderComponent->mCollider.axis[1] = axis.pRows[1];
		collider->mColliderComponent->mCollider.axis[2] = axis.pRows[2];

		collider->mColliderComponent->mLayer = layer;
	}
}

void loadDoors(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " doors...");
	for (auto obj : *objs)
	{
		auto canOpen = obj["canOpen"].get<bool>();
		if (!canOpen) continue;
		auto position = parseVec3f(obj["position"]);
		auto rotation = parseQuatf(obj["rotation"]);
		auto scale = parseVec3f(obj["scale"]);
		auto collOrigin = parseVec3f(obj["bounds"]["center"]);
		auto collHalf = parseVec3f(obj["bounds"]["extents"]);

		auto door = Factory<Door>::Create();
		parseTransform(obj, door->mTransform);
		Resource::mModelManager->GetModel("Door")->Link(door);

		door->mTransform->SetPosition(position);
		door->mTransform->SetRotation(rotation);
		door->mTransform->SetScale(scale);

		door->mColliderComponent->mCollider.origin = collOrigin;
		door->mColliderComponent->mCollider.origin.z = -7.5f;
		door->mColliderComponent->mCollider.halfSize = collHalf;
		door->mColliderComponent->mCollider.halfSize.z = 7.5f;
		door->mColliderComponent->mCollider.axis[0] = vec3f(1, 0, 0);
		door->mColliderComponent->mCollider.axis[1] = vec3f(0, 1, 0);
		door->mColliderComponent->mCollider.axis[2] = vec3f(0, 0, 1);
		door->mColliderComponent->mLayer = COLLISION_LAYER_WALL;

		door->mTrigger->mCollider.axis[0] = vec3f(1, 0, 0);
		door->mTrigger->mCollider.axis[1] = vec3f(0, 1, 0);
		door->mTrigger->mCollider.axis[2] = vec3f(0, 0, 1);
		door->mTrigger->mCollider.origin = door->mColliderComponent->mCollider.origin;
		door->mTrigger->mCollider.halfSize = door->mColliderComponent->mCollider.halfSize;
		if (door->mTrigger->mCollider.halfSize.x < door->mTrigger->mCollider.halfSize.y)
			door->mTrigger->mCollider.halfSize.x = 3;
		else
			door->mTrigger->mCollider.halfSize.y = 3;
		door->mTrigger->mLayer = COLLISION_LAYER_DOOR;
	}
}


ModelManager* Resource::mModelManager;

Resource::LevelInfo Resource::LoadLevel(string path, LinearAllocator& allocator)
{
	mModelManager = Application::SharedInstance().GetModelManager();
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

	auto lamps = obj["lamps"].get_ptr<jarr_t>();
	if (lamps != nullptr)
	{
		loadLamps(lamps);

		level.lampWorldMatrices = reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * lamps->size(), alignof(mat4f), 0));
		level.lampVPTMatrices	= reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * lamps->size(), alignof(mat4f), 0));
		level.lampCount			= static_cast<short>(lamps->size());

		mat4f defaultRotation	= mat4f::rotateZ(PI * 0.5f);
		vec3f defaultUp			= { 0.0f, 0.0f, -1.0f };

		int i = 0;
		for (Lamp& l : Factory<Lamp>())
		{
			level.lampWorldMatrices[i] = (mat4f::scale(l.mLightRadius) * defaultRotation * l.mTransform->GetWorldMatrix()).transpose();
			
			// Note: we don't bother transposing this matrix because we need it for culling in row maj fashion.
			level.lampVPTMatrices[i] = 
				mat4f::lookToLH(l.mLightDirection, l.mTransform->GetPosition(), defaultUp) 
				* mat4f::normalizedPerspectiveLH(l.mLightAngle, 1.0f, 0.1f, l.mLightRadius);
			i++;
		}
	}

	auto domination = obj["dominationPoints"].get_ptr<jarr_t>();
	if (domination != nullptr)
	{
		loadDomPoints(domination);
	}

	auto pickups = obj["pickups"].get_ptr<jarr_t>();
	if (pickups != nullptr)
	{
		loadPickups(pickups);
	}

	auto spawnPoints = obj["spawnPoints"].get_ptr<jarr_t>();
	if (spawnPoints != nullptr)
	{
		loadSpawnPoints(spawnPoints);
	}

	auto doors = obj["doors"].get_ptr<jarr_t>();
	if (doors != nullptr)
	{
		loadDoors(doors);
	}

	auto staticMeshes = obj["staticMeshes"];
	if (staticMeshes != nullptr)
	{
		
		vector<string> l_textureNames;
		int m = 0;
		for (json::iterator it = staticMeshes.begin(); it != staticMeshes.end(); ++it) {
			std::cout << it.key() << " : " << it.value() << "\n";

			auto model = it.key();
			auto meshes = obj["staticMeshes"][it.key()].get_ptr<jarr_t>();
			if (meshes) {
				loadStaticMeshes(meshes, model, l_textureNames, level.materialIDs);
				level.staticMeshCount += static_cast<short>(meshes->size());
			}
		}

		for (string s : l_textureNames)
		{
			level.textureNames.push_back("Assets/Textures/StaticMesh/" + s);
		}

		level.staticMeshWorldMatrices = reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * level.staticMeshCount, alignof(mat4f), 0));

		int i = 0;
		for (StaticMesh& w : Factory<StaticMesh>()) {
			level.staticMeshWorldMatrices[i++] = w.mTransform->GetWorldMatrix().transpose();
		}
	}

	// Set wall, and floor mesh colliders inactive.

	vector<StaticMeshModel> inactiveStaticMeshModels = {
		STATIC_MESH_MODEL_CORNER_WALL,
		STATIC_MESH_MODEL_CURVED_WALL,
		STATIC_MESH_MODEL_DOOR_BLOCKED,
		STATIC_MESH_MODEL_FLOOR,
		STATIC_MESH_NORMAL_WALL,
		STATIC_MESH_T_WALL,
		STATIC_MESH_MODEL_TRI_WALL,
		STATIC_MESH_MODEL_WALL,
		STATIC_MESH_MODEL_D_DOOR_WALL,
		STATIC_MESH_MODEL_S_DOOR_WALL,
		STATIC_MESH_MODEL_S_WINDOW_WALL,
		STATIC_MESH_MODEL_WINDOW_WALL
	};

	for (StaticMeshModel smm : inactiveStaticMeshModels)
	{
		vector<BaseSceneObject*>* pStaticMeshes = mModelManager->RequestAllUsingModel(kStaticMeshModelNames[smm]);
		for (unsigned int i = 0; i < pStaticMeshes->size(); i++)
		{
			StaticMesh* pStaticMesh = reinterpret_cast<StaticMesh*>((*pStaticMeshes)[i]);
			pStaticMesh->mColliderComponent->mIsActive = false;
		}
	}

	auto regions = obj["regions"].get_ptr<jarr_t>();
	if (regions != nullptr)
	{
		level.regionCount += static_cast<short>(regions->size());
		loadRegions(regions, COLLISION_LAYER_FLOOR, level.center, level.extents);
	}

	auto ramps = obj["ramps"].get_ptr<jarr_t>();
	if (ramps != nullptr)
	{
		level.regionCount += static_cast<short>(ramps->size());
		loadRegions(ramps, COLLISION_LAYER_FLOOR, level.center, level.extents);
	}

	auto colliders = obj["staticColliders"].get_ptr<jarr_t>();
	if (colliders != nullptr)
	{
		level.staticColliderCount = static_cast<short>(colliders->size());
		loadStaticColliders(colliders, COLLISION_LAYER_WALL, level.center, level.extents);
	}

	return level;
}

