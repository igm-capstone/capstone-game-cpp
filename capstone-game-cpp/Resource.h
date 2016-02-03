#pragma once
 
#define TILE_COUNT_X 2
#define TILE_COUNT_Y 2

class Resource
{
	struct LevelInfo
	{
		mat4f* walls;
		mat4f* tiles;
		mat4f* lamps;
		vec4f* lampColors;
		Rig3D::Transform* moveableBlocks;

		vec3f center, extents;
		float tileWidth, tileHeight;
		short wallCount;
		short tileCount;
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

