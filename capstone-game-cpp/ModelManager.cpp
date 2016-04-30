#include "stdafx.h"
#include "ModelManager.h"
#include "Vertex.h"
#include <algorithm>

const uint32_t kStaticMeshModelTextureCount = 18;

const char* kSprinterModelName		= "Sprinter_Final";
const char* kTrapperModelName		= "Trapmaster_Final";
const char* kProfessorModelName		= "ProfMayor";
const char* kMinionAnimModelName	= "Minion_Test";
const char* kPlantModelName			= "FlyTrap";

const char* kDoorModelName			= "Door";
const char* kTrapModelName			= "TrapAnimated";

const char* kStaticMeshModelNames[STATIC_MESH_MODEL_COUNT] =
{
	"BathroomCabinet",
	"Bathtub",
	"bedside drawers",
	"BigBox",
	"Bookshelf",
	"bookshelf_curved",
	"bookshelf_flat",
	"Bookstack_01",
	"Bookstack_02",
	"BottleBig",
	"BottleSmall",
	"Chair",
	"Couch",
	"CurvedStair",
	"CurvedStairsLeft",
	"CurvedWall",
	"DiningChair",
	"DiningTable",
	"DoorBlocked",
	"Fireplace",
	"Floor",
	"Generator",
	"LightCannon",
	"OfficeChair",
	"OfficeDesk",
	"OutsideFloor",
	"OwnerBed",
	"Pillar",
	"RectangularTable",
	"RoundTable",
	"Sink",
	"SmallBox",
	"StaffBed",
	"Stair_Full",
	"Stair_Half",
	"StairHalf_Railing",
	"Stairs_Full_Railing",
	"StudyChair",
	"T_Wall",
	"Toilet",
	"TriangleWall",
	"Wall",
	"Wall_Corner",
	"Wall_DoubleDoor",
	"Wall_SingleDoor",
	"Wall_StairConnector",
	"Wall_Window",
	"WallLantern",
	"Wardrobe"
};

void ModelCluster::Unlink(BaseSceneObject* obj)
{
	mObjects.erase(std::remove(mObjects.begin(), mObjects.end(), obj), mObjects.end());
	obj->mModel = nullptr;
}

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