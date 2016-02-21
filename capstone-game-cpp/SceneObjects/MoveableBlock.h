#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>

class MoveableBlock : public BaseSceneObject
{
	friend class Factory<MoveableBlock>;

public:
	BoxColliderComponent*	mBoxCollider;

private:
	MoveableBlock() : 
		mBoxCollider(Factory<BoxColliderComponent>::Create())
	{
		mBoxCollider->mSceneObject = this;
	}

	~MoveableBlock() {};
};