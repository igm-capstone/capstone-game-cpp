#pragma once
#include "BaseSceneObject.h"
#include <Components/NetworkID.h>
#include <Components/ColliderComponent.h>
#include <Components/MinionController.h>

class Minion : public BaseSceneObject
{
	friend class Factory<Minion>;

public:
	IMesh*						mMesh;
	NetworkID*					mNetworkID;
	MinionController*			mController;
	SphereColliderComponent*	mCollider;

private:
	Minion()
	{
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;
		mNetworkID->OnNetAuthorityChange(&OnNetAuthorityChange);

		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;
		mCollider->mIsDynamic = true;

		mController = Factory<MinionController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;
		mController->RegisterMoveCallback(&OnMove);
	}

	~Minion();

public:
	void Spawn(vec3f pos, int UUID)
	{
		mTransform->SetPosition(pos);

		mCollider->mIsActive = true;
		mCollider->mCollider.origin = pos;

		mNetworkID->mIsActive = true;
		mNetworkID->mUUID = UUID;
	};

	static void OnMove(BaseSceneObject* obj, vec3f newPos)
	{
		auto m = static_cast<Minion*>(obj);
		m->mTransform->SetPosition(newPos);
		m->mCollider->mCollider.origin = newPos;
	}

	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
	{
		auto m = static_cast<Minion*>(obj);
		m->mController->mIsActive = newAuth;
	}

};

