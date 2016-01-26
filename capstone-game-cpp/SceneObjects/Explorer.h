#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"

class Explorer : public BaseSceneObject
{
	friend class FactoryPool<Explorer>;
public:
	IMesh*		mMesh;
	NetworkID*	mNetworkID;
	
	union {
		BoxCollider*	mBoxCollider;
		SphereCollider* mSphereCollider;
	};

private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mNetworkID = FactoryPool<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
	}
	~Explorer() {};
};