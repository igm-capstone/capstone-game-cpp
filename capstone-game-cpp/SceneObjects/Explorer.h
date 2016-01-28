#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Pool.h"
#include <Components/ColliderComponent.h>

class Explorer : public BaseSceneObject
{
	friend class Pool<Explorer>;
public:
	IMesh*		mMesh;
	NetworkID*	mNetworkID;
	
	union {
		BoxColliderComponent*	mBoxCollider;
		SphereColliderComponent* mSphereCollider;
	};

private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mNetworkID = Pool<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
	}
	~Explorer() {};
};