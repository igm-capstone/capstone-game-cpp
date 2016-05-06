#include "stdafx.h"
#include "Region.h"

Region::Region() : mColliderComponent(Factory<OrientedBoxColliderComponent>::Create())
{
	mColliderComponent->mLayer = COLLISION_LAYER_FLOOR;
	mColliderComponent->mIsTrigger = false;
	mColliderComponent->mIsDynamic = false;
	mColliderComponent->mIsActive = true;
	mColliderComponent->mSceneObject = this;
}

Region::~Region()
{
	Factory<OrientedBoxColliderComponent>::Destroy(mColliderComponent);
}
