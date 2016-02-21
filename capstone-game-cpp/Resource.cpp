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
		transform->SetRotation(parseQuatf(rotation)*quatf::rollPitchYaw(0, -0.5*PI, -PI));
		//transform->SetRotation(quatf::rollPitchYaw(0, 0, 0.5*PI)*parseQuatf(rotation));
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
	
	vec3f defaultDirection = { 1.0f, 0.0f, 0.0f };

	float radians = PI / 180.0f;

	for (auto obj : *objs)
	{
		auto lamp = Factory<Lamp>::Create();
		parseTransform(obj, lamp->mTransform);
		
		lamp->mLightColor		= { 0.6f, 0.6f, 0.1f, 1.0f };
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


void loadStaticMeshes(jarr_t objs, std::string model)
{
	TRACE_LOG("Loading " << int(objs->size()) << " static meshes...");
	for (auto obj : *objs)
	{
		auto staticMesh = Factory<StaticMesh>::Create();
		parseTransform(obj, staticMesh->mTransform);
		Resource::mModelManager->GetModel(model.c_str())->Link(staticMesh);
	}
}


void loadStaticColliders(jarr_t objs)
{
	TRACE_LOG("Loading " << int(objs->size()) << " static colliders...");
	for (auto obj : *objs)
	{
		auto collider = Factory<StaticCollider>::Create();
		parseTransform(obj, collider->mTransform);
		collider->mBoxCollider->mCollider.origin = collider->mTransform->GetPosition();
		collider->mBoxCollider->mCollider.halfSize = collider->mTransform->GetScale() * 0.5f;

		mat3f axis = collider->mTransform->GetRotationMatrix();
		collider->mBoxCollider->mCollider.axis[0] = axis.pRows[0];
		collider->mBoxCollider->mCollider.axis[1] = axis.pRows[1];
		collider->mBoxCollider->mCollider.axis[2] = axis.pRows[2];

		collider->mBoxCollider->mLayer = static_cast<CLayer>(obj["layer"].get<short>());
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

	level.floorCount = TILE_COUNT_X * TILE_COUNT_Y;
	level.floorWorldMatrices = reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * level.floorCount, alignof(mat4f), 0));

	float levelWidth	= level.extents.x * 2.0f;
	float levelHeight	= level.extents.y * 2.0f;

	level.floorWidth		=	levelWidth / static_cast<float>(TILE_COUNT_X);
	level.floorHeight	=	levelHeight / static_cast<float>(TILE_COUNT_Y);
	
	float halfWidth		= (levelWidth - level.floorWidth) * 0.5f;
	float halfHeight	= (levelHeight - level.floorHeight) * 0.5f;

	for (int y = 0; y < TILE_COUNT_Y; y++)
	{
		for (int x = 0; x < TILE_COUNT_X; x++)
		{
			level.floorWorldMatrices[y * TILE_COUNT_X + x] = (mat4f::rotateX(-PI * 0.5f) * mat4f::translate({ level.center.x + (x * level.floorWidth - halfWidth) , level.center.y + (halfHeight - y * level.floorHeight), 0.5f })).transpose(); //FIXME: hardcoded z value
		}
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

	auto moveableBlocks = obj["moveableBlocks"].get_ptr<jarr_t>();
	if (moveableBlocks != nullptr)
	{
		loadBlocks(moveableBlocks);
	}

	auto staticMeshes = obj["staticMeshes"];
	if (staticMeshes != nullptr)
	{
		for (json::iterator it = staticMeshes.begin(); it != staticMeshes.end(); ++it) {
			std::cout << it.key() << " : " << it.value() << "\n";

			auto model = it.key();
			auto meshes = obj["staticMeshes"][it.key()].get_ptr<jarr_t>();
			if (model == "Floor") continue;
			if (meshes) {
				loadStaticMeshes(meshes, model);
				level.staticMeshCount += static_cast<short>(meshes->size());
			}
		}

		level.staticMeshWorldMatrices = reinterpret_cast<mat4f*>(allocator.Allocate(sizeof(mat4f) * level.staticMeshCount, alignof(mat4f), 0));

		int i = 0;
		for (StaticMesh& w : Factory<StaticMesh>()) {
			level.staticMeshWorldMatrices[i++] = w.mTransform->GetWorldMatrix().transpose();
		}
	}

	auto colliders = obj["colliders"].get_ptr<jarr_t>();
	if (colliders != nullptr)
	{
		level.staticColliderCount = static_cast<short>(colliders->size());
		loadStaticColliders(colliders);
	}

	return level;
}

