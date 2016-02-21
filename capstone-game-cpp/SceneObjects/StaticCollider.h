#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"
#include "Components/ColliderComponent.h"

class StaticCollider : public BaseSceneObject
{

	friend class Factory<StaticCollider>;

public:
	class OrientedBoxColliderComponent* mBoxCollider;

private:
	StaticCollider() : mBoxCollider(Factory<OrientedBoxColliderComponent>::Create())
	{
		mBoxCollider->mIsTrigger	= false;
		mBoxCollider->mIsDynamic	= false;
		mBoxCollider->mIsActive		= true;

		mBoxCollider->mSceneObject	= this;
	};
	
	~StaticCollider() {};
};