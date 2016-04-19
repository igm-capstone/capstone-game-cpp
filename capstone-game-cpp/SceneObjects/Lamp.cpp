#include "stdafx.h"
#include "Lamp.h"
#include "Explorer.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Components/NetworkID.h>


Lamp::Lamp()
	: mConeMesh(nullptr)
	, mNetworkManager(&Singleton<NetworkManager>::SharedInstance())
	, mLightRadius(0.0f)
	, mLightAngle(0.0f)
	, mStatus(LAMP_OFF)
{
	mTrigger = Factory<OrientedBoxColliderComponent>::Create(),
	mTrigger->mIsTrigger = true;
	mTrigger->mIsDynamic = false;
	mTrigger->mIsActive = true;
	mTrigger->mSceneObject = this;
	mTrigger->RegisterTriggerStayCallback(OnTriggerStay);

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mHasAuthority = mNetworkManager->mMode == NetworkManager::SERVER;
	mNetworkID->RegisterInteractCallback(&OnInteract);
}

Lamp::~Lamp()
{
	Factory<OrientedBoxColliderComponent>::Destroy(mTrigger);
}

void Lamp::OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
{
	Lamp* lamp = static_cast<Lamp*>(obj);
	if (other->Is<Explorer>())
	{
		Explorer* e = static_cast<Explorer*>(other);
		if (e->mController->mIsInteracting) {
			e->mController->ConsumeInteractWill();
			lamp->Interact();
		}
	}
}


void Lamp::OnInteract(BaseSceneObject* obj)
{
	auto lamp = reinterpret_cast<Lamp*>(obj);
	lamp->ToggleLamp();
}


void Lamp::SetID(uint16_t id)
{
	mNetworkID->mUUID = LAMP_BASE_UUID + id;
}


void Lamp::Interact()
{
	Packet p(INTERACT);
	p.UUID = mNetworkID->mUUID;

	ToggleLamp();
	if (mNetworkID->mHasAuthority)
	{
		mNetworkManager->mServer.SendToAll(&p);
	}
	else
	{
		mNetworkManager->mClient.SendData(&p);
	}
}

void Lamp::ToggleLamp()
{
	switch (mStatus)
	{
	case LAMP_OFF:
		mStatus = LAMP_ON;
		break;
	case LAMP_ON:
		mStatus = LAMP_DIMMED;
		break;
	case LAMP_DIMMED:
		mStatus = LAMP_ON;
		break;
	}
}
