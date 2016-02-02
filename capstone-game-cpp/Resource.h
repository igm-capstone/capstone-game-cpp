#pragma once
 
class Resource
{
	struct LevelInfo
	{
		mat4f* walls;
		mat4f* lamps;
		vec4f* lampColors;
		Rig3D::Transform* moveableBlocks;

		short wallCount;
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

