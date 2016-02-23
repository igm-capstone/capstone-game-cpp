#include "stdafx.h"
#include "Region.h"

Region::Region() : mColliderComponent(Factory<OrientedBoxColliderComponent>::Create())
{
	mColliderComponent->mLayer = COLLISION_LAYER_FLOOR;
	mColliderComponent->mIsTrigger = false;
	mColliderComponent->mIsDynamic = false;
	mColliderComponent->mIsActive = true;
	mColliderComponent->mSceneObject = this;

	mPlaneCollider.normal = { 0.0f, 0.0f, -1.0f };
	mPlaneCollider.distance = 0.0f;
}

Region::~Region()
{

}
