#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"
#include "Components/ColliderComponent.h"

class StaticCollider : public BaseSceneObject
{

	friend class Factory<StaticCollider>;

public:
	class OrientedBoxColliderComponent* mColliderComponent;

private:
	StaticCollider() : mColliderComponent(Factory<OrientedBoxColliderComponent>::Create())
	{
		mColliderComponent->mIsTrigger	= false;
		mColliderComponent->mIsDynamic	= false;
		mColliderComponent->mIsActive		= true;

		mColliderComponent->mSceneObject	= this;
	};
	
	~StaticCollider()
	{
		Factory<OrientedBoxColliderComponent>::Destroy(mColliderComponent);
	};
};