#include "TargetFollower.h"

using namespace Rig3D;


TargetFollower::TargetFollower(Transform& transform, AABB<vec2f>* aabbs, int aabbbCount) : 
	mGrid(Grid::SharedInstance()), 
	mTransform(transform),
	mAABBs(aabbs), 
	mAABBCount(aabbbCount),
	mReverseTime(0), 
	mRepel(0)
{
}


TargetFollower::~TargetFollower()
{
}
