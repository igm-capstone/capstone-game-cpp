#pragma once
#include "BaseSceneObject.h"
#include <Pool.h>

class Wall : public BaseSceneObject
{
	friend class Factory<Wall>;

public:
	IMesh*		mMesh;

	union {
		BoxCollider*	mBoxCollider;
		SphereCollider* mSphereCollider;
	};

private:
	Wall() : mMesh(nullptr) {
		//mNetworkID = Factory<NetworkID>::Create();
		//mNetworkID->mSceneObject = this;
	}
	~Wall() {};
};