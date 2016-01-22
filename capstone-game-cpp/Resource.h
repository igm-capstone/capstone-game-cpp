#pragma once
 
class Resource
{
	struct LevelInfo
	{
		mat4f* walls;
		mat4f* moveableBlocks;
	};

public:
	static LevelInfo LoadLevel(std::string path, LinearAllocator& allocator);

private:
	Resource(Resource const&) = delete;
	void operator=(Resource const&) = delete;
	Resource() = delete;
	~Resource() = delete;
};

