#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Pool.h"

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	IMesh*		mMesh;
	NetworkID*	mNetworkID;
	
	union {
		BoxCollider*	mBoxCollider;
		SphereCollider* mSphereCollider;
	};

private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
	}
	~Explorer() {};
};