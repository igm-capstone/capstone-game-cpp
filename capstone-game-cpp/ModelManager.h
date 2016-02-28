#pragma once
#include "SkeletalHierarchy.h"
#include <algorithm>
#include "SceneObjects/BaseSceneObject.h"
#include "FBXResource.h"

enum StaticMeshModel : int
{
	STATIC_MESH_MODEL_WALL,
	STATIC_MESH_MODEL_TRI_WALL,
	STATIC_MESH_MODEL_D_DOOR_WALL,
	STATIC_MESH_MODEL_S_DOOR_WALL,
	STATIC_MESH_MODEL_S_WINDOW_WALL,
	STATIC_MESH_MODEL_CURVED_WALL,
	STATIC_MESH_MODEL_FLOOR,
	STATIC_MESH_MODEL_COUNT
};

extern const char* kMinionAnimModelName;
extern const char* kDoorModelName;

extern const char* kStaticMeshModelNames[STATIC_MESH_MODEL_COUNT];

class ModelCluster
{
	friend class ModelManager;
public:
	const char * mName;
	class Rig3D::IMesh* mMesh;
	uint32_t			mMaterialIndex;		// Index of the SRV within a shader resource
	std::vector<SkeletalAnimation> mSkeletalAnimations;
	SkeletalHierarchy mSkeletalHierarchy;

private:
	std::vector<BaseSceneObject*> mObjects;

public:
	void Link(BaseSceneObject * obj) { mObjects.push_back(obj); obj->mModel = this; }
	//erase-remove idiom https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	void Unlink(BaseSceneObject * obj) { mObjects.erase(remove(mObjects.begin(), mObjects.end(), obj), mObjects.end()); obj->mModel = nullptr; }
	int ShareCount() { return mObjects.size(); }
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
			c = static_cast<ModelCluster*>(mAllocator->Allocate(sizeof(ModelCluster), alignof(ModelCluster), 0));
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



