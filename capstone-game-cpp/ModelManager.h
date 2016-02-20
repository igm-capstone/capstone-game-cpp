#pragma once
#include "SkeletalHierarchy.h"
#include <algorithm>
#include "SceneObjects/BaseSceneObject.h"

class ModelCluster
{
	friend class ModelManager;
public:
	class Rig3D::IMesh* mMesh;
	std::vector<SkeletalAnimation> mSkeletalAnimations;
	SkeletalHierarchy mSkeletalHierarchy;

private:
	std::vector<BaseSceneObject*> mObjects;

public:
	void Link(BaseSceneObject * obj) { mObjects.push_back(obj); obj->mModel = this; }
	//erase-remove idiom https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	void Unlink(BaseSceneObject * obj) { mObjects.erase(remove(mObjects.begin(), mObjects.end(), obj), mObjects.end()); obj->mModel = nullptr; }
};


class ModelManager
{
	Rig3D::Renderer* mRenderer;
	LinearAllocator* mAllocator;
	Rig3D::MeshLibrary<LinearAllocator> mMeshLibrary;

	std::unordered_map<const char*, ModelCluster*> mModelMap;

public:
	ModelManager();
	ModelManager(ModelManager const&) = delete;
	void operator=(ModelManager const&) = delete;
	~ModelManager();
	void SetAllocator(LinearAllocator* allocator);

	ModelCluster* RequestModel(const char* name);
	std::vector<BaseSceneObject*>* RequestAllUsingModel(const char* name);
};



