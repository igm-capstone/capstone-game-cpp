#pragma once
#include <SkeletalHierarchy.h>
#include <trace.h>
#include <unordered_map>

template <class Vertex>
class BINMeshResource
{
public:
	std::unordered_map<int, std::vector<JointBlendWeight>>	mControlPointJointBlendMap;
	std::vector<Vertex>										mVertices;
	std::vector<uint16_t>									mIndices;

	SkeletalHierarchy										mSkeletalHierarchy;
	std::vector<SkeletalAnimation>							mSkeletalAnimations;

	const char*		mFilename;
	void*	mImporter;

	BINMeshResource(const char* filename) : mFilename(filename), mImporter(nullptr)
	{

	}

	BINMeshResource() : BINMeshResource(nullptr)
	{
			
	}

	~BINMeshResource()
	{
		mControlPointJointBlendMap.clear();
		mVertices.clear();
		mIndices.clear();
		mSkeletalAnimations.clear();
	}

	int Load()
	{
		FILE* file;
		fopen_s(&file, mFilename, "r");
		fread((void*)this, sizeof(*this), 1, file);
		fclose(file);

		return 1;
	}
};
