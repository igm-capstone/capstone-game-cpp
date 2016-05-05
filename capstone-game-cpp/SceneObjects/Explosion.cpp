#include "stdafx.h"
#include "Explosion.h"
#include "Components/ColliderComponent.h"
#include "Components/NetworkID.h"
#include "Explorer.h"
#include "Components/Health.h"
#include <trace.h>


Explosion::Explosion() 
	: mSphereColliderComponent(Factory<SphereColliderComponent>::Create())
	, mNetworkID(Factory<NetworkID>::Create())
	, mExplosionDamage(50.0f)
	, mDuration(0.0f)
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

Explosion::~Explosion()
{
	Factory<SphereColliderComponent>::Destroy(mSphereColliderComponent);
	Factory<NetworkID>::Destroy(mNetworkID);
}

void Explosion::Spawn(vec3f pos, int UUID, float duration)
{
	mTransform->SetPosition(pos);
	mSphereColliderComponent->mCollider.origin = pos;
	mNetworkID->mUUID = UUID;
	mDuration = duration;
}

void Explosion::Update(float seconds)
{
	static float growthRate = 2.0f * seconds;
	vec3f scale = mTransform->GetScale();

	if (scale.x <= mSphereColliderComponent->mCollider.radius)
	{
		vec3f s = vec3f(cliqCity::graphicsMath::lerp(1.0f, mSphereColliderComponent->mCollider.radius, 1.0f - mDuration));
		mTransform->SetScale(s);
	}

	mDuration -= seconds;
	if (mDuration <= 0.0f)
	{
		Factory<Explosion>::Destroy(this);
	}
}

void Explosion::OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other)
{
	Explorer* pExlorer = reinterpret_cast<Explorer*>(other);
	pExlorer->mHealth->SetHealth(pExlorer->mHealth->GetHealth() - reinterpret_cast<Explosion*>(self)->mExplosionDamage);
}