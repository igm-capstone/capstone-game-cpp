#include "stdafx.h"
#include "Door.h"
#include "Explorer.h"
#include "Network/NetworkManager.h"
#include <Components/NetworkID.h>
#include <Components/ExplorerController.h>
#include <Components/ColliderComponent.h>


Door::Door()
	: mNetworkManager(&Singleton<NetworkManager>::SharedInstance())
{

	mColliderComponent = Factory<OrientedBoxColliderComponent>::Create();
	mColliderComponent->mSceneObject = this;
	mColliderComponent->mIsTrigger = false;
	mColliderComponent->mIsDynamic = false;
	mColliderComponent->mIsActive = true;

	mTrigger = Factory<OrientedBoxColliderComponent>::Create();
	mTrigger->mSceneObject = this;
	mTrigger->mIsTrigger = true;
	mTrigger->mIsDynamic = false;
	mTrigger->mIsActive = true;
	mTrigger->RegisterTriggerStayCallback(OnTriggerStay);
	

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mHasAuthority = mNetworkManager->mMode == NetworkManager::SERVER;
	mNetworkID->RegisterInteractCallback(&OnInteract);

}


Door::~Door()
{
	Factory<OrientedBoxColliderComponent>::Destroy(mColliderComponent);
	Factory<OrientedBoxColliderComponent>::Destroy(mTrigger);
	Factory<NetworkID>::Destroy(mNetworkID);
}


void Door::OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
{
	Door* door = static_cast<Door*>(obj);
	if (other->Is<Explorer>())
	{
		Explorer* e = static_cast<Explorer*>(other);
		if (e->mController->mIsInteracting) {
			e->mController->ConsumeInteractWill();
			door->Interact();
		}
	}
}


void Door::OnInteract(BaseSceneObject* obj)
{
	auto door = reinterpret_cast<Door*>(obj);
	door->ToggleDoor();
}


void Door::SetID(uint16_t id)
{
	mNetworkID->mUUID = DOOR_BASE_UUID + id;
}


void Door::Interact()
{
	Packet p(INTERACT);
	p.UUID = mNetworkID->mUUID;

	ToggleDoor();
	if (mNetworkID->mHasAuthority)
	{
		mNetworkManager->mServer.SendToAll(&p);
	}
	else
	{
		mNetworkManager->mClient.SendData(&p);
	}
}


void Door::ToggleDoor()
{
	mColliderComponent->mIsActive ? mTransform->RotateRoll(0.5f*PI) : mTransform->RotateRoll(-0.5f*PI);
	mColliderComponent->mIsActive = !mColliderComponent->mIsActive;
}
