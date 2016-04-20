#include "stdafx.h"
#include "Heal.h"
#include "Components/ColliderComponent.h"
#include "Components/NetworkID.h"
#include "Explorer.h"
#include "Components/Health.h"
#include <trace.h>


Heal::Heal() : mSphereColliderComponent(Factory<SphereColliderComponent>::Create()), mNetworkID(Factory<NetworkID>::Create()), mHealthRestored(50.0f), mDuration(0.0f)
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

Heal::~Heal()
{
	Factory<SphereColliderComponent>::Destroy(mSphereColliderComponent);
	Factory<NetworkID>::Destroy(mNetworkID);
}

void Heal::Spawn(vec3f pos, int UUID, float duration)
{
	mTransform->SetPosition(pos);
	mSphereColliderComponent->mCollider.origin = pos;
	mNetworkID->mUUID = UUID;
	mDuration = duration;
}

void Heal::Update(float seconds)
{
	static float growthRate = 2.0f * seconds;
	vec3f scale = mTransform->GetScale();

	if (scale.x <= mSphereColliderComponent->mCollider.radius)
	{
		vec3f s = vec3f(cliqCity::graphicsMath::lerp(1.0f, mSphereColliderComponent->mCollider.radius, 1.0f - mDuration));
		mTransform->SetScale(s);
	}
}

void Heal::OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other)
{
	Explorer* pExlorer = reinterpret_cast<Explorer*>(other);
	pExlorer->mHealth->SetHealth(pExlorer->mHealth->GetHealth() + reinterpret_cast<Heal*>(self)->mHealthRestored);
}