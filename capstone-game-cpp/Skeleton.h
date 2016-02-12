#pragma once
#include <stdint.h>
#include <Rig3D/GraphicsMath/cgm.h>
#include <assert.h>
#include <vector>

struct Joint
{
	mat4f		mInverseBindPose;
	int			mParentIndex;		// -1 indicates root 
	const char* mName;
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
};


