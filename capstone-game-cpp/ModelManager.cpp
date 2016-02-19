#include "stdafx.h"
#include "ModelManager.h"
#include "Vertex.h"
#include "FBXResource.h"


ModelManager::ModelManager()
{
	mRenderer = Singleton<Engine>::SharedInstance().GetRenderer();
}


ModelManager::~ModelManager()
{
	for each (auto c in mModelMap)
	{
		c.second->mMesh->~IMesh();
	}
}

void ModelManager::SetAllocator(LinearAllocator* allocator)
{
	mAllocator = allocator;
	mMeshLibrary.SetAllocator(allocator);
}

ModelCluster* ModelManager::RequestModel(const char* name)
{
	ModelCluster* c = mModelMap[name];
	if (!c)
	{
		c = static_cast<ModelCluster*>(mAllocator->Allocate(sizeof(ModelCluster), alignof(ModelCluster), 0));
		char filename[40];
		sprintf_s(filename, "Assets/%s.fbx", name);
		FBXMeshResource<SkinnedVertex> fbxResource(filename);
		mMeshLibrary.LoadMesh(&c->mMesh, mRenderer, fbxResource);
		c->mSkeletalHierarchy = fbxResource.mSkeletalHierarchy;
		c->mSkeletalAnimations = fbxResource.mSkeletalAnimations;

		mModelMap[name] = c;
	}

	return c;
}

std::vector<BaseSceneObject*>* ModelManager::RequestAllUsingModel(const char* name)
{
	ModelCluster* c = mModelMap[name];
	if (!c) return nullptr;
	return &c->mObjects;
}