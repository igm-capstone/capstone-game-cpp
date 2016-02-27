#include "stdafx.h"
#include "ModelManager.h"
#include "Vertex.h"

const char* kMinionAnimModelName = "Minion_Test";
const char* kDoorModelName       = "Door";

const char* kStaticMeshModelNames[STATIC_MESH_MODEL_COUNT] =
{
	"Wall",
	"TriangleWall",
	"Wall_DoubleDoor",
	"Wall_SingleDoor",
	"Wall_W_SingleWindwo",
	"CurvedWall",
	"Floor"
};


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

ModelCluster* ModelManager::GetModel(std::string name)
{
	ModelCluster* c = mModelMap[name];
	assert(c != nullptr); //Model not loaded, try LoadModel first!
	return c;
}

std::vector<BaseSceneObject*>* ModelManager::RequestAllUsingModel(std::string name)
{
	ModelCluster* c = mModelMap[name];
	assert(c != nullptr); //Model not loaded, try LoadModel first!
	return &c->mObjects;
}