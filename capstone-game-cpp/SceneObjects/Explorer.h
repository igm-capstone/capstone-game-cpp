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
		mController->mIsActive = false;

		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mTraits.isDynamic = true;
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;
	}
	~Explorer() {};

public:
	void Spawn(vec3f pos, int UUID)
	{
		mTransform->SetPosition(pos);

		mCollider->mIsActive = true;
		mCollider->mCollider.origin = pos;

		mNetworkID->mIsActive = true;
		mNetworkID->mUUID = UUID;
	};

	void OnNetAuthorityChange(bool newAuth)
	{
		mController->mIsActive = newAuth;
	}

	void OnNetSyncTransform(vec3f newPos)
	{
		mTransform->SetPosition(newPos);
		mCollider->mCollider.origin = newPos;
	}
};
