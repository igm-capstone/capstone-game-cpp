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


	struct BinHeader1
	{
		// timestamp??
		int32_t mIndexCount;
		int32_t mVertexCount;
	};

	int Load()
	{
		ifstream ifs(mFilename, ifstream::in | ifstream::binary);
		if (!ifs.is_open())
		{
			TRACE_ERROR("Error opening file " << mFilename);
		};
		// get size of file
		//infile.seekg(0, infile.end);
		//long size = infile.tellg();
		//infile.seekg(0);

		//BinHeader1 header;
		char* buffer = new char[1];
		if (!ifs.read(buffer, 1))
		{
			TRACE_ERROR("file error");
		};

		ifs.close();
		delete buffer;
		return 1;

		FILE* file;
		fopen_s(&file, mFilename, "r");
		fread((void*)this, sizeof(*this), 1, file);
		fclose(file);

		return 1;
	}

};
