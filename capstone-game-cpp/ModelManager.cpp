#include "stdafx.h"
#include "ModelManager.h"
#include "Vertex.h"

const uint32_t kStaticMeshModelTextureCount = 18;

const char* kMinionAnimModelName	= "Minion_Test";
const char* kDoorModelName			= "Door";
const char* kLampModelName			= "WallLantern";

const char* kStaticMeshModelNames[STATIC_MESH_MODEL_COUNT] =
{
	"BathroomCabinet",
	"Bathtub",
	"Bookshelf",
	"BookshelfCurved",
	"BookshelfFlat",
	"BottleBig",
	"BottleSmall",
	"Chair",
	"Corner_Wall",
	"CurvedWall",
	"Couch",
	"DiningChair",
	"DiningTable",
	"DomPoint",
	"DoorBlocked",
	"Floor",
	"Lever",
	"LightCannon",
	"NormalWall",
	"RampPrototype04x02x02",
	"Sink",
	"StepsPrototype04x02x02",
	"T_Wall",
	"Toilet",
	"TriangleWall",
	"Wall",
	"Wall_DoubleDoor",
	"Wall_SingleDoor",
	"Wall_W_SingleWindwo",
	"Wall_Window",
	"WallLantern",
	"Wardrobe"
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