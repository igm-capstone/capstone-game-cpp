#pragma once
#include "BaseSceneObject.h"
#include <Pool.h>
#include <Components/ColliderComponent.h>

class Wall : public BaseSceneObject
{
	friend class Factory<Wall>;

public:
	IMesh*		mMesh;
	BoxColliderComponent*	mBoxCollider;

private:
	Wall() : mMesh(nullptr) {
		mBoxCollider = Factory<BoxColliderComponent>::Create();
		mBoxCollider->mSceneObject = this;

		//mNetworkID = Factory<NetworkID>::Create();
		//mNetworkID->mSceneObject = this;
	}
	~Wall() {};
};