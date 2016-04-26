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
		ifstream ifs(mFilename, ifstream::binary);
		if (!ifs.is_open())
		{
			return 1;
			TRACE_ERROR("Error opening file " << mFilename);
		};

		FILETIME lastWriteTime;
		ifs.read(reinterpret_cast<char*>(&lastWriteTime), sizeof(FILETIME));

		size_t indexCount, vertexCount;

		ifs.read(reinterpret_cast<char*>(&indexCount), sizeof(size_t));
		mIndices.resize(indexCount);
		ifs.read(reinterpret_cast<char*>(&mIndices[0]), sizeof(uint16_t) * indexCount);

		ifs.read(reinterpret_cast<char*>(&vertexCount), sizeof(size_t));
		mVertices.resize(vertexCount);
		ifs.read(reinterpret_cast<char*>(&mVertices[0]), sizeof(Vertex) * vertexCount);

		size_t mapSize, index, count;
		ifs.read(reinterpret_cast<char*>(&mapSize), sizeof(size_t));

		if (!mapSize)
		{
			ifs.close();
			return 1;
		}

		for (size_t i = 0; i < mapSize; i++)
		{
			ifs.read(reinterpret_cast<char*>(&index), sizeof(size_t));
			ifs.read(reinterpret_cast<char*>(&count), sizeof(size_t));

			auto& v = mControlPointJointBlendMap[index];
			v.resize(count);
			ifs.read(reinterpret_cast<char*>(&v[0]), sizeof(JointBlendWeight) * count);
		}

		size_t skeletalSize;
		ifs.read(reinterpret_cast<char*>(&skeletalSize), sizeof(size_t));

		auto& joints = mSkeletalHierarchy.mJoints;
		joints.resize(skeletalSize);

		for (size_t i = 0; i < skeletalSize; i++)
		{
			size_t nameSize;
			ifs.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));

			char buff[100];
			ifs.read(buff, nameSize);
			joints[i].name = string(buff).c_str();

			ifs.read(reinterpret_cast<char*>(&joints[i].inverseBindPoseMatrix), sizeof(mat4f));
			ifs.read(reinterpret_cast<char*>(&joints[i].animPoseMatrix), sizeof(mat4f));
			ifs.read(reinterpret_cast<char*>(&joints[i].parentIndex), sizeof(int));
		}

		size_t skeletalAnimSize;
		ifs.read(reinterpret_cast<char*>(&skeletalAnimSize), sizeof(size_t));

		mSkeletalAnimations.resize(skeletalAnimSize);
		for (size_t i = 0; i < skeletalAnimSize; i++)
		{
			mSkeletalAnimations[i].Deserialize(ifs);
		}

		ifs.close();
		return 1;
	}

};
