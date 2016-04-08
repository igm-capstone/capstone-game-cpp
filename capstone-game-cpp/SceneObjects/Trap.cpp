#include "stdafx.h"
#include "Trap.h"
#include "Components/ColliderComponent.h"
#include "Factory.h"
#include "Components/NetworkID.h"
#include "StatusEffect.h"

Trap::Trap() : mSphereColliderComponent(Factory<SphereColliderComponent>::Create()), mNetworkID(Factory<NetworkID>::Create()), mEffect(Factory<StatusEffect>::Create()), mDuration(5.0f)
{
	mSphereColliderComponent->mCollider.radius = 5.0f;
	mSphereColliderComponent->mLayer = COLLISION_LAYER_EXPLORER_SKILL;
	mSphereColliderComponent->mIsTrigger = true;
	mSphereColliderComponent->mIsDynamic = false;
	mSphereColliderComponent->mSceneObject = this;
	mSphereColliderComponent->RegisterTriggerEnterCallback(OnTriggerEnter);

	mNetworkID->mIsActive = false;
	mNetworkID->mSceneObject = this;
}

Trap::~Trap()
{
	Factory<SphereColliderComponent>::Destroy(mSphereColliderComponent);
	Factory<NetworkID>::Destroy(mNetworkID);
}

void Trap::Spawn(int UUID, vec3f position, float duration)
{
	mTransform->SetPosition(position);
	mSphereColliderComponent->mCollider.origin = position;
	mNetworkID->mUUID = UUID;
	mDuration = duration;
}

void Trap::OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other)
{
	Trap* pTrap = reinterpret_cast<Trap*>(self);
	
	if (other->Is<Explorer>())
	{
		Explorer* pExplorer = reinterpret_cast<Explorer*>(other);
		if (pTrap->mEffect->mExplorers.find(pExplorer) == pTrap->mEffect->mExplorers.end())
		{
			pTrap->mEffect->mExplorers[pExplorer] = pTrap->mEffect->mDuration;
		}
	}
	else if (other->Is<Minion>())
	{
		Minion* pMinion = reinterpret_cast<Minion*>(other);
		if (pTrap->mEffect->mMinions.find(pMinion) == pTrap->mEffect->mMinions.end())
		{
			pTrap->mEffect->mMinions[pMinion] = pTrap->mEffect->mDuration;
		}
	}
}
