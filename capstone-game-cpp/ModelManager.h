#pragma once
#include "SkeletalHierarchy.h"
#include "SceneObjects/BaseSceneObject.h"
#include "FBXResource.h"
#include "BINResource.h"
#include "Windows.h"

enum StaticMeshModel : int
{
	STATIC_MESH_MODEL_BATHROOM_CABINET,
	STATIC_MESH_MODEL_BATHTUB,
	STATIC_MESH_MODEL_BEDSIDE_DRAWER,
	STATIC_MESH_MODEL_BIGBOX,
	STATIC_MESH_MODEL_BOOKSHELF,
	STATIC_MESH_MODEL_BOOKSHELF_CURVED,
	STATIC_MESH_MODEL_BOOKSHELF_FLAT,
	STATIC_MESH_MODEL_BOOKSTACK_0,
	STATIC_MESH_MODEL_BOOKSTACK_1,
	STATIC_MESH_MODEL_BOTTLE_BIG,
	STATIC_MESH_MODEL_BOTTLE_SMALL,
	STATIC_MESH_MODEL_CHAIR,
	STATIC_MESH_MODEL_COUCH,
	STATIC_MESH_MODEL_CURVED_STAIRS,
	STATIC_MESH_MODEL_CURVED_STAIRS_LEFT,
	STATIC_MESH_MODEL_CURVED_WALL,
	STATIC_MESH_MODEL_DINING_CHAIR,
	STATIC_MESH_MODEL_DINING_TABLE,
	STATIC_MESH_MODEL_DOOR_BLOCKED,
	STATIC_MESH_MODEL_FIREPLACE,
	STATIC_MESH_MODEL_FLOOR,
	STATIC_MESH_MODEL_GENERATOR,
	STATIC_MESH_MODEL_LIGHT_CANNON,
	STATIC_MESH_MODEL_OFFICE_CHAIR,
	STATIC_MESH_MODEL_OFFICE_DESK,
	STATIC_MESH_MODEL_OUTSIDE_FLOOR,
	STATIC_MESH_MODEL_OWNER_BED,
	STATIC_MESH_MODEL_PILLAR,
	STATIC_MESH_MODEL_RECT_TABLE,
	STATIC_MESH_MODEL_ROUND_TABLE,
	STATIC_MESH_SINK,
	STATIC_MESH_SMALL_BOX,
	STATIC_MESH_STAFF_BED,
	STATIC_MESH_STAIR_FULL,
	STATIC_MESH_STAIR_HALF,
	STATIC_MESH_STAIR_HALF_RAILING,
	STATIC_MESH_STAIR_FULL_RAILING,
	STATIC_MESH_STUDY_CHAIR,
	STATIC_MESH_T_WALL,
	STATIC_MESH_TOILET,
	STATIC_MESH_MODEL_TRI_WALL,
	STATIC_MESH_MODEL_WALL,
	STATIC_MESH_MODEL_CORNER_WALL,
	STATIC_MESH_MODEL_D_DOOR_WALL,
	STATIC_MESH_MODEL_S_DOOR_WALL,
	STATIC_MESH_MODEL_WALL_CONN,
	STATIC_MESH_MODEL_WINDOW_WALL,
	STATIC_MESH_WALL_LANTERN,
	STATIC_MESH_WARDROBE,
	STATIC_MESH_MODEL_COUNT
};

extern const char* kSprinterModelName;
extern const char* kTrapperModelName;
extern const char* kProfessorModelName;
extern const char* kMinionAnimModelName;
extern const char* kPlantModelName;
extern const char* kDoorModelName;
extern const char* kTrapModelName;
extern const char* kLanternModelName;

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

	FILETIME GetFileTimestamp(std::string filename)
	{
		HANDLE file;
		//file = CreateFile(LPCWSTR(filename.c_str()),
		file = CreateFile(std::wstring(filename.begin(), filename.end()).c_str(),
			GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
			return{ 0L, 0L };
		}

		FILETIME lastWriteTime;
		if (!GetFileTime(file, nullptr, nullptr, &lastWriteTime))
		{
			return{ 0L, 0L };
		}

		CloseHandle(file);

		return lastWriteTime;
	}

	FILETIME GetBinFileTimestamp(std::string filename)
	{
		std::ifstream ifs(filename, std::ifstream::binary);
		if (!ifs.is_open())
		{
			return{ 0L, 0L };
		}

		FILETIME lastWriteTime;
		ifs.read(reinterpret_cast<char*>(&lastWriteTime), sizeof(FILETIME));

		ifs.close();

		return lastWriteTime;
	}

	bool CompareFilestamps(FILETIME lhs, FILETIME rhs)
	{
		return lhs.dwHighDateTime == rhs.dwHighDateTime && lhs.dwLowDateTime == rhs.dwLowDateTime;
	}

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
		if (c)
		{
			return c;
		}

		void* ptr = mAllocator->Allocate(sizeof(ModelCluster), alignof(ModelCluster), 0);
		c = new(ptr) ModelCluster();

		std::string filePath = "Assets/Models/" + std::string(name);
		std::string fbxPath = filePath + ".fbx";
		std::string binPath = filePath + ".bin";

		FILETIME fbxTimestamp = GetFileTimestamp(fbxPath);
		FILETIME binTimestamp = GetBinFileTimestamp(binPath);

		if (CompareFilestamps(fbxTimestamp, binTimestamp)) {
			//We got a binary cache of the model
			BINMeshResource<Vertex> binResource(binPath.c_str());
			mMeshLibrary.LoadMesh(&c->mMesh, mRenderer, binResource);
			c->mSkeletalHierarchy = binResource.mSkeletalHierarchy;
			c->mSkeletalAnimations = binResource.mSkeletalAnimations;
		}
		else
		{
			//Load FBX
			FBXMeshResource<Vertex> fbxResource(fbxPath.c_str());
			mMeshLibrary.LoadMesh(&c->mMesh, mRenderer, fbxResource);
			c->mSkeletalHierarchy = fbxResource.mSkeletalHierarchy;
			c->mSkeletalAnimations = fbxResource.mSkeletalAnimations;

			fbxResource.SaveToBin(fbxTimestamp);
		}

		c->mName = name;
			
		mModelMap[name] = c;

		return c;
	}
};



