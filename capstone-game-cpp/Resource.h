#pragma once
 
#define TILE_COUNT_X 2
#define TILE_COUNT_Y 2

class Resource
{
	struct LevelInfo
	{
		mat4f* staticMeshWorldMatrices;
		mat4f* floorWorldMatrices;
		mat4f* lampWorldMatrices;
		mat4f* lampVPTMatrices;
		Rig3D::Transform* moveableBlocks;

		vec3f center, extents;
		float floorWidth, floorHeight;
		short staticMeshCount;
		short floorCount;
		short lampCount;
	};

public:
	static LevelInfo LoadLevel(std::string path, LinearAllocator& allocator);

private:
	Resource(Resource const&) = delete;
	void operator=(Resource const&) = delete;
	Resource() = delete;
	~Resource() = delete;
};

