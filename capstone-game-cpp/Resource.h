#pragma once
 
class Resource
{
	struct LevelInfo
	{
		mat4f* walls;
		Rig3D::Transform* moveableBlocks;

		short wallCount;
	};

public:
	static LevelInfo LoadLevel(std::string path, LinearAllocator& allocator);

private:
	Resource(Resource const&) = delete;
	void operator=(Resource const&) = delete;
	Resource() = delete;
	~Resource() = delete;
};

