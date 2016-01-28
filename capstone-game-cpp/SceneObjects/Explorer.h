#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	IMesh*					mMesh;
	NetworkID*				mNetworkID;
	ExplorerController*		mController;
	BoxColliderComponent*	mCollider;


private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<ExplorerController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;

		mCollider = Factory<BoxColliderComponent>::Create();
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;
	}
	~Explorer() {};
};
