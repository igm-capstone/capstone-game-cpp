#include "stdafx.h"
#include "Trap.h"
#include "Components/ColliderComponent.h"
#include "Factory.h"
#include "Components/NetworkID.h"


Trap::Trap() : mSphereColliderComponent(Factory<SphereColliderComponent>::Create()), mNetworkID(Factory<NetworkID>::Create()), mDuration(0.0f)
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
	
}
