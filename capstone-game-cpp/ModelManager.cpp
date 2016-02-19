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
}

void ModelManager::SetAllocator(LinearAllocator* allocator)
{
	mAllocator = allocator;
	mMeshLibrary.SetAllocator(allocator);
}

ModelCluster* ModelManager::AddModel(const char* name)
{
	ModelCluster* c;
	if (mModelMap.count(name) == 0)
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
	else
	{
		c = mModelMap[name];
	}
	return c;
}

ModelCluster* ModelManager::RequestModel(const char* name)
{
	return mModelMap[name];
}