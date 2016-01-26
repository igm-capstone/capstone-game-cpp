#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"


class Explorer : public BaseSceneObject
{
public:
	IMesh*		mMesh;
	NetworkID*	mNetworkID;

	union {
		BoxCollider*	mBoxCollider;
		SphereCollider* mSphereCollider;
	};

	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		//FIXME: factory stuff
		mNetworkID = new NetworkID();
	}
	~Explorer() {};
};