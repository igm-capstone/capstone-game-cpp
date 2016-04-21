#pragma once
#include "SkeletalHierarchy.h"
#include <algorithm>
#include "SceneObjects/BaseSceneObject.h"
#include "FBXResource.h"

enum StaticMeshModel : int
{
	STATIC_MESH_MODEL_BATHROOM_CABINET,
	STATIC_MESH_MODEL_BATHTUB,
	STATIC_MESH_MODEL_BOOKSHELF,
	STATIC_MESH_MODEL_BOOKSHELF_CURVED,
	STATIC_MESH_MODEL_BOOKSHELF_FLAT,
	STATIC_MESH_MODEL_BOTTLE_BIG,
	STATIC_MESH_MODEL_BOTTLE_SMALL,
	STATIC_MESH_MODEL_CHAIR,
	STATIC_MESH_MODEL_CORNER_WALL,
	STATIC_MESH_MODEL_CURVED_WALL,
	STATIC_MESH_MODEL_COUCH,
	STATIC_MESH_MODEL_DINING_CHAIR,
	STATIC_MESH_MODEL_DINING_TABLE,
	STATIC_MESH_MODEL_DOM_POINT,
	STATIC_MESH_MODEL_DOOR_BLOCKED,
	STATIC_MESH_MODEL_FLOOR,
	STATIC_MESH_MODEL_LEVER,
	STATIC_MESH_MODEL_LIGHT_CANNON,
	STATIC_MESH_NORMAL_WALL,
	STATIC_MESH_RAMP_422,
	STATIC_MESH_SINK,
	STATIC_MESH_STEPS_422,
	STATIC_MESH_T_WALL,
	STATIC_MESH_TOILET,
	STATIC_MESH_MODEL_TRI_WALL,
	STATIC_MESH_MODEL_WALL,
	STATIC_MESH_MODEL_D_DOOR_WALL,
	STATIC_MESH_MODEL_S_DOOR_WALL,
	STATIC_MESH_MODEL_S_WINDOW_WALL,
	STATIC_MESH_MODEL_WINDOW_WALL,
	STATIC_MESH_WALL_LANTERN,
	STATIC_MESH_WARDROBE,
	STATIC_MESH_MODEL_COUNT
};

extern const char* kSprinterModelName;
extern const char* kMinionAnimModelName;
extern const char* kPlantModelName;
extern const char* kDoorModelName;
extern const char* kTrapModelName;

extern const char* kStaticMeshModelNames[STATIC_MESH_MODEL_COUNT];

class ModelCluster
{
	friend class ModelManager;
public:
	const char * mName;
	class Rig3D::IMesh* mMesh;
	std::vector<SkeletalAnimation> mSkeletalAnimations;
	SkeletalHierarchy mSkeletalHierarchy;

private:
	std::vector<BaseSceneObject*> mObjects;

public:
	ModelCluster() : mName(nullptr), mMesh(nullptr)
	{
		
	}

	void Link(BaseSceneObject * obj)
	{
		mObjects.push_back(obj); 
		obj->mModel = this;
	}

	//erase-remove idiom https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	void Unlink(BaseSceneObject * obj);

	int ShareCount()
	{
		return mObjects.size();
	}
};

class ModelManager
{
	Rig3D::Renderer* mRenderer;
	LinearAllocator* mAllocator;
	Rig3D::MeshLibrary<LinearAllocator> mMeshLibrary;

	std::unordered_map<std::string, ModelCluster*> mModelMap;

public:
	ModelManager();
	ModelManager(ModelManager const&) = delete;
	void operator=(ModelManager const&) = delete;
	~ModelManager();
	void SetAllocator(LinearAllocator* allocator);
	ModelCluster* GetModel(std::string name);
	std::vector<BaseSceneObject*>* RequestAllUsingModel(std::string name);
	

	template <class Vertex>
	ModelCluster* LoadModel(const char* name)
	{
		ModelCluster* c = mModelMap[name];
		if (!c)
		{
			void* ptr = mAllocator->Allocate(sizeof(ModelCluster), alignof(ModelCluster), 0);
			c = new(ptr) ModelCluster();

			std::string path = "Assets/Models/" + std::string(name) + ".fbx";
			FBXMeshResource<Vertex> fbxResource(path.c_str());
			mMeshLibrary.LoadMesh(&c->mMesh, mRenderer, fbxResource);
			c->mSkeletalHierarchy = fbxResource.mSkeletalHierarchy;
			c->mSkeletalAnimations = fbxResource.mSkeletalAnimations;
			c->mName = name;

			mModelMap[name] = c;
		}

		return c;
	}
};



