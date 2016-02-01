#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>

#define MAX_EXPLORERS 4

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	IMesh*						mMesh;
	NetworkID*					mNetworkID;
	ExplorerController*			mController;
	SphereColliderComponent*	mCollider;


private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<ExplorerController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = true;

		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mTraits.isDynamic = true;
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;
	}
	~Explorer() {};
};
