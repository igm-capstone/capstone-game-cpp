#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>

class Wall : public BaseSceneObject
{
	friend class Factory<Wall>;

public:
	IMesh*							mMesh;
	OrientedBoxColliderComponent*	mBoxCollider;

private:
	Wall() : 
		mMesh(nullptr),
		mBoxCollider(Factory<OrientedBoxColliderComponent>::Create())
	{
		mBoxCollider->mTraits.isDynamic = false;	// Walls don't move :)
		mBoxCollider->mSceneObject = this;
	}

	~Wall() {};
};