#pragma once
 
#define TILE_COUNT_X 2
#define TILE_COUNT_Y 2
#include "ModelManager.h"

class Resource
{
public:	
	struct LevelInfo
	{
		mat4f* staticMeshWorldMatrices;
		mat4f* lampWorldMatrices;
		mat4f* lampVPTMatrices;
		Rig3D::Transform* moveableBlocks;

		vec3f center, extents;

		short staticColliderCount	= 0;
		short regionCount			= 0;
		short staticMeshCount		= 0;
		short lampCount				= 0;

		std::vector<std::string>	textureNames;
		std::vector<uint32_t>		materialIDs;
	};

public:
	static LevelInfo LoadLevel(std::string path, LinearAllocator& allocator);
	static ModelManager* mModelManager;

private:
	Resource(Resource const&) = delete;
	void operator=(Resource const&) = delete;
	Resource() = delete;
	~Resource() = delete;
};

