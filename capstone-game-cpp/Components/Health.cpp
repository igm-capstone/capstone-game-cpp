#include "stdafx.h"
#include "Health.h"
#include "NetworkID.h"
#include <SceneObjects/Explorer.h>
#include <SceneObjects/Minion.h>
#include <trace.h>

void Health::OnSyncHealth(BaseSceneObject* obj, float newVal, float newMax)
{
	Health* h;
	if (obj->Is<Explorer>())
	{
		h = static_cast<Explorer*>(obj)->mHealth;
	}
	else if (obj->Is<Minion>())
	{
		h = static_cast<Minion*>(obj)->mHealth;
	}
	else
	{
		TRACE_WARN("??? helth is being used by something other than explarer or minion, please fix OnSyncHealth");
		return;
	}


	TRACE_LOG("sync health " << newVal << " " << newMax);
	if (newVal >= 0)
	{
		auto oldVal = h->mCurrentHealth;
		h->mCurrentHealth = newVal;
		
		if (newVal == 0 && oldVal != 0)
		{
			h->OnHealthToZero();
		}
		else if (newVal != 0 && oldVal == 0)
		{
			h->OnHealthFromZero();
		}
		else if (newVal != oldVal)
		{
			h->OnHealthChange(newVal);
		}

	}

	if (newMax >= 0)
	{
		h->mMaxHealth = newMax;
	}
}


void Health::SetupNetworkID(NetworkID* networkID)
{
	mNetworkID = networkID;
	mNetworkID->RegisterNetHealthChangeCallback(&OnSyncHealth);
}

void Health::SetMaxHealth(float val)
{
	if (val < 1)
	{
		mCurrentHealth = 1;
		mMaxHealth = 1;
	}
	else
	{
		mCurrentHealth = val;
		mMaxHealth = val;
	}

	SyncHealth(true);
}

void Health::TakeDamage(float i, bool checkAuthority)
{
	SetHealth(mCurrentHealth - i, checkAuthority);
}

void Health::SyncHealth(bool checkAuthority)
{
	if (mNetworkID && (!checkAuthority || mNetworkID->mHasAuthority))
	{
		Packet p(PacketTypes::SYNC_HEALTH);
		p.UUID = mNetworkID->mUUID;
		p.AsFloatArray[0] = mCurrentHealth;
		p.AsFloatArray[1] = mMaxHealth;

		if (mNetworkManager.mMode == NetworkManager::SERVER)
		{
			mNetworkManager.mServer.SendToAll(&p);
		}
		else
		{
			mNetworkManager.mClient.SendData(&p);
		}
	}
}

void Health::SetHealth(float val, bool checkAuthority)
{
	float prevHealth = mCurrentHealth;
	mCurrentHealth = val;

	if (mCurrentHealth < 0)
	{
		mCurrentHealth = 0;
	}
	else if (mCurrentHealth > mMaxHealth)
	{
		mCurrentHealth = mMaxHealth;
	}
	
	SyncHealth(checkAuthority);

	if (prevHealth > 0.0f && mCurrentHealth == 0.0f)
	{
		OnHealthToZero();
	}
	else if (prevHealth == 0.0f && mCurrentHealth > 0.0f)
	{
		OnHealthFromZero();
	}
	else if (mCurrentHealth != prevHealth)
	{
		OnHealthChange(val);
	}
}
