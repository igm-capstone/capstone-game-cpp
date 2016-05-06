#include "stdafx.h"
#include "Lantern.h"
#include "Components/ColliderComponent.h"
#include "Components/NetworkID.h"
#include "SceneObjects/StatusEffect.h"

Lantern::Lantern() 
{
	mTransform->SetRotation(-PI * 0.5f, 0.0f, 0.0f);

	mLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	mZOffset = -3.0f;
	mShouldDestroy = false;

	mColliderComponent = Factory<SphereColliderComponent>::Create();
	mColliderComponent->mCollider.radius = 5.0f;
	mColliderComponent->mLayer = COLLISION_LAYER_EXPLORER_SKILL;
	mColliderComponent->mIsTrigger = true;
	mColliderComponent->mIsDynamic = false;
	mColliderComponent->mSceneObject = this;
	mColliderComponent->mIsActive = false;
	mColliderComponent->RegisterTriggerStayCallback(OnTriggerStay);

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mIsActive = false;
	mNetworkID->mSceneObject = this;

	mStatusEffect = Factory<StatusEffect>::Create();
	mStatusEffect->mIsActive = false;

}

Lantern::~Lantern()
{
	Factory<SphereColliderComponent>::Destroy(mColliderComponent);
	Factory<NetworkID>::Destroy(mNetworkID);
	Factory<StatusEffect>::Destroy(mStatusEffect);
}

void Lantern::Spawn(int UUID, vec3f position, float duration)
{
	mTransform->SetPosition(position);
	mColliderComponent->mCollider.origin = position;
	mColliderComponent->mIsActive = true;
	mNetworkID->mUUID = UUID;
	mDuration = duration;

	mStatusEffect->mIsActive = true;
	mStatusEffect->mOnEnterCallback = StatusEffect::OnSlowEnter;
	mStatusEffect->mOnUpdateCallback = StatusEffect::OnSlowUpdate;
	mStatusEffect->mOnDestroyCallback = StatusEffect::OnSlowDestroy;
	mStatusEffect->mFlavorText = "SLOWED";
	mStatusEffect->mDuration = duration;
}

void Lantern::Update(float seconds)
{
	mDuration -= seconds;
	if (mDuration < 0.0f)
	{
		mShouldDestroy = true;
	}
}

void Lantern::OnTriggerStay(BaseSceneObject* self, BaseSceneObject* other)
{
	Lantern* pLantern = reinterpret_cast<Lantern*>(self);

	if (other->Is<Minion>())
	{
		Minion* pMinion = reinterpret_cast<Minion*>(other);
		if (pLantern->mStatusEffect->mMinions.find(pMinion) == pLantern->mStatusEffect->mMinions.end())
		{
			pLantern->mStatusEffect->mMinions[pMinion] = pLantern->mStatusEffect->mDuration;
		}
	}

	if (pLantern->mStatusEffect->mOnEnterCallback)
	{
		pLantern->mStatusEffect->mOnEnterCallback(pLantern->mStatusEffect, other);
	}
}
