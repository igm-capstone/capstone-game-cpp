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
	mat4f		inverseBindPoseMatrix;  // Transforms from model space bind pose to joint space bind pose
	mat4f		animPoseMatrix;			// Transforms from joint space bind pose to model space animated pose
	int			parentIndex;			// -1 indicates root 
	const char* name;
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

	Skeleton(const Skeleton& other)
	{
		mJoints = other.mJoints;
	}

	Skeleton(Skeleton&& other)
	{
		mJoints = other.mJoints;

		other.mJoints.clear();
	}

	Skeleton& operator=(const Skeleton& other)
	{
		mJoints = other.mJoints;

		return *this;
	}

	Skeleton& operator=(Skeleton&& other)
	{
		mJoints = other.mJoints;

		other.mJoints.clear();

		return *this;
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

	inline void UpdateAnimationPose()
	{
		Joint* joints		= &mJoints[0];
		uint32_t jointCount = mJoints.size();

		for (uint32_t i = 1; i < jointCount; i++)
		{
			joints[i].animPoseMatrix = joints[i].animPoseMatrix * joints[joints[i].parentIndex].animPoseMatrix;
		}
	}

	void CalculateSkinningMatrices(mat4f* skinningMatrices)
	{
		Joint* joints		= &mJoints[0];
		uint32_t jointCount	= mJoints.size();

		for (uint32_t i = 0; i < jointCount; i++)
		{
			skinningMatrices[i] = (joints[i].inverseBindPoseMatrix * joints[i].animPoseMatrix).transpose();
		}
	}
};


