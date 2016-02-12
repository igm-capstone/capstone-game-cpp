#pragma once
#include <stdint.h>
#include <Rig3D/GraphicsMath/cgm.h>
#include <assert.h>
#include <vector>

struct JointBlendWeight
{
	int		jointIndex;
	float	jointWeight;
};

struct Joint
{
	mat4f		inverseBindPoseMatrix;
	int			parentIndex;		// -1 indicates root 
	const char* name;
};

struct SkeletalAnimation
{
	const char* name;
};

struct Keyframe
{
	Joint* joint;
};

class Skeleton
{
public:
	std::vector<Joint> mJoints;

	Skeleton()
	{
		
	}

	~Skeleton()
	{
		mJoints.clear();
	}

	Joint* GetJointByName(const char* name)
	{
		for (Joint joint : mJoints)
		{
			if (strcmp(name, joint.name) == 0)
			{
				return &joint;
			}
		}

		return nullptr;
	}

	int GetJointIndexByName(const char* name)
	{
		for (uint32_t i = 0; i < mJoints.size(); i++)
		{
			if (strcmp(name, mJoints[i].name) == 0)
			{
				return static_cast<int>(i);
			}
		}

		return  -1;
	}
};


