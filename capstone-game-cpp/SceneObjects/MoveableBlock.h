#pragma once
#include "BaseSceneObject.h"
#include <Pool.h>
#include <Components/ColliderComponent.h>

class MoveableBlock : public BaseSceneObject
{
	friend class Factory<MoveableBlock>;

public:
	IMesh*		mMesh;
	BoxColliderComponent*	mBoxCollider;

private:
	MoveableBlock() : 
		mMesh(nullptr), 
		mBoxCollider(Factory<BoxColliderComponent>::Create())
	{
		mBoxCollider->mSceneObject = this;
	}

	~MoveableBlock() {};
};