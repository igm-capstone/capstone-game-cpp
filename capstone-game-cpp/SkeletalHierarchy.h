#pragma once
#include <stdint.h>
#include <Rig3D/GraphicsMath/cgm.h>
#include <vector>

struct JointBlendWeight
{
	int		jointIndex;
	float	jointWeight;
};

struct Joint
{
	mat4f		inverseBindPoseMatrix;  // Transforms from model space bind pose to joint space bind pose
	mat4f		animPoseMatrix;			// Transforms from joint space bind pose to model space animated pose
	int			parentIndex;			// -1 indicates root 
	std::string name;

	void Serialize(std::ostream& os)
	{
		uint32_t nameSize = name.size();
		os.write(reinterpret_cast<char*>(&nameSize), sizeof(uint32_t));
		os.write(name.c_str(), sizeof(char*) * nameSize);
		os.write(reinterpret_cast<char*>(&inverseBindPoseMatrix), sizeof(mat4f));
		os.write(reinterpret_cast<char*>(&animPoseMatrix), sizeof(mat4f));
		os.write(reinterpret_cast<char*>(&parentIndex), sizeof(int));
	}

	void Deserialize(std::istream& is)
	{
		uint32_t nameSize;
		is.read(reinterpret_cast<char*>(&nameSize), sizeof(uint32_t));
		
		char buff[100];
		is.read(buff, nameSize);
		buff[nameSize] = 0;
		name = std::string(buff);

		is.read(reinterpret_cast<char*>(&inverseBindPoseMatrix), sizeof(mat4f));
		is.read(reinterpret_cast<char*>(&animPoseMatrix), sizeof(mat4f));
		is.read(reinterpret_cast<char*>(&parentIndex), sizeof(int));
	}
};

struct Keyframe
{
	quatf rotation;
	vec3f scale;
	vec3f translation;
	float time;
};

struct JointAnimation
{
	uint32_t jointIndex;
	std::vector<Keyframe> keyframes;

	void Serialize(std::ostream& os)
	{
		os.write(reinterpret_cast<char*>(&jointIndex), sizeof(uint32_t));

		size_t size = keyframes.size();
		os.write(reinterpret_cast<char*>(&size), sizeof(size_t));
		os.write(reinterpret_cast<char*>(&keyframes[0]), sizeof(Keyframe) * size);
	}

	void Deserialize(std::istream& is)
	{
		is.read(reinterpret_cast<char*>(&jointIndex), sizeof(uint32_t));

		size_t size;
		is.read(reinterpret_cast<char*>(&size), sizeof(size_t));
		
		keyframes.resize(size);
		is.read(reinterpret_cast<char*>(&keyframes[0]), sizeof(Keyframe) * size);
	}
};

struct SkeletalAnimation
{
	uint32_t	frameCount;
	float		duration;
	std::string name;
	std::vector<JointAnimation> jointAnimations;

	void Serialize(std::ostream& os)
	{
		os.write(reinterpret_cast<char*>(&frameCount), sizeof(uint32_t));
		os.write(reinterpret_cast<char*>(&duration), sizeof(float));
		
		size_t nameSize = name.length();
		os.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
		os.write(name.c_str(), nameSize);

		size_t size = jointAnimations.size();
		os.write(reinterpret_cast<char*>(&size), sizeof(size_t));
		for (auto jointAnim : jointAnimations)
		{
			jointAnim.Serialize(os);
		}
	}

	void Deserialize(std::istream& is)
	{
		is.read(reinterpret_cast<char*>(&frameCount), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&duration), sizeof(float));

		size_t nameSize;
		is.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));

		char buff[100];
		is.read(buff, nameSize);
		buff[nameSize] = 0;
		name = std::string(buff);

		size_t size;
		is.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		jointAnimations.resize(size);
		for (size_t i = 0; i < size; i++)
		{
			jointAnimations[i].Deserialize(is);
		}
	}
};

class SkeletalHierarchy
{
public:
	std::vector<Joint> mJoints;

	SkeletalHierarchy();
	~SkeletalHierarchy();

	SkeletalHierarchy(const SkeletalHierarchy& other);
	SkeletalHierarchy(SkeletalHierarchy&& other);

	SkeletalHierarchy& operator=(const SkeletalHierarchy& other);
	SkeletalHierarchy& operator=(SkeletalHierarchy&& other);

	Joint*	GetJointByName(const char* name);
	int		GetJointIndexByName(const char* name);

	void	CalculateSkinningMatrices(mat4f* skinningMatrices);
};


