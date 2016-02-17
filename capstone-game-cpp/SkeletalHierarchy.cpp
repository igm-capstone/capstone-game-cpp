#include <stdafx.h>
#include <SkeletalHierarchy.h>

SkeletalHierarchy::SkeletalHierarchy()
{

}

SkeletalHierarchy::~SkeletalHierarchy()
{
	mJoints.clear();
}

SkeletalHierarchy::SkeletalHierarchy(const SkeletalHierarchy& other)
{
	mJoints = other.mJoints;
}

SkeletalHierarchy::SkeletalHierarchy(SkeletalHierarchy&& other)
{
	mJoints = other.mJoints;

	other.mJoints.clear();
}

SkeletalHierarchy& SkeletalHierarchy::operator=(const SkeletalHierarchy& other)
{
	mJoints = other.mJoints;

	return *this;
}

SkeletalHierarchy& SkeletalHierarchy::operator=(SkeletalHierarchy&& other)
{
	mJoints = other.mJoints;

	other.mJoints.clear();

	return *this;
}

Joint* SkeletalHierarchy::GetJointByName(const char* name)
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

int SkeletalHierarchy::GetJointIndexByName(const char* name)
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

void SkeletalHierarchy::UpdateAnimationPose()
{
	Joint* joints = &mJoints[0];
	uint32_t jointCount = mJoints.size();

	for (uint32_t i = 1; i < jointCount; i++)
	{
		joints[i].animPoseMatrix = joints[i].animPoseMatrix * joints[joints[i].parentIndex].animPoseMatrix;
	}
}

void SkeletalHierarchy::CalculateSkinningMatrices(mat4f* skinningMatrices)
{
	Joint* joints = &mJoints[0];
	uint32_t jointCount = mJoints.size();

	for (uint32_t i = 0; i < jointCount; i++)
	{
		skinningMatrices[i] = (joints[i].inverseBindPoseMatrix * joints[i].animPoseMatrix).transpose();
	}
}