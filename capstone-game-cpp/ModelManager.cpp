#include "stdafx.h"
#include "ModelManager.h"
#include "Vertex.h"

const char* kWallModelName				= "Wall";
const char* kTriangleWallModelName		= "TriangleWall";
const char* kWallDoubleDoorModelName	= "Wall_DoubleDoor";
const char* kWallSingleDoorModelName	= "Wall_SingleDoor";
const char* kWallSingleWindowModelName	= "Wall_W_SingleWindwo";
const char* kCurvedWallModelName		= "CurvedWall";
const char* kFloorModelName				= "Floor";
const char* kDoorModelName				= "Door";
const char* kMinionAnimModelName		= "Minion_Test";

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
	assert(c != nullptr); //Model not load, try LoadModel first!
	return c;
}

std::vector<BaseSceneObject*>* ModelManager::RequestAllUsingModel(std::string name)
{
	ModelCluster* c = mModelMap[name];
	assert(c != nullptr); //Model not load, try LoadModel first!
	return &c->mObjects;
}