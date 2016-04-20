#include "stdafx.h"
#include "Trap.h"
#include "Components/ColliderComponent.h"
#include "Factory.h"
#include "Components/NetworkID.h"
#include "StatusEffect.h"
#include <trace.h>
#include <ModelManager.h>
#include <Components/AnimationController.h>
#include <ScareTacticsApplication.h>
#include <Components/AnimationUtility.h>

Trap::Trap() :
	mSphereColliderComponent(Factory<SphereColliderComponent>::Create()),
//	mAnimationController(Factory<AnimationController>::Create()),
	mNetworkID(Factory<NetworkID>::Create()),
	mEffect(Factory<StatusEffect>::Create()),
	mDuration(5.0f),
	mEffectDuration(5.0f),
	mDelay(2.0f),
	mShouldDestroy(false)
{
	mTransform->SetRotation(-PI * 0.5f, 0.0f, 0.0f);

	mSphereColliderComponent->mCollider.radius = 5.0f;
	mSphereColliderComponent->mLayer = COLLISION_LAYER_EXPLORER_SKILL;
	mSphereColliderComponent->mIsTrigger = true;
	mSphereColliderComponent->mIsDynamic = false;
	mSphereColliderComponent->mSceneObject = this;
	mSphereColliderComponent->mIsActive = false;
	mSphereColliderComponent->RegisterTriggerEnterCallback(OnTriggerEnter);
	mSphereColliderComponent->RegisterTriggerStayCallback(OnTriggerStay);

	mNetworkID->mIsActive = false;
	mNetworkID->mSceneObject = this;

	Application::SharedInstance().GetModelManager()->GetModel(kTrapModelName)->Link(this);

	//mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	//mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;
	//mAnimationController->mSceneObject = this;

	//Animation open = gTrapAnimations[Animations::TRAP_OPEN];
	//KeyframeOption openOptions[] = {
	//	{ open.endFrameIndex,   &OnTrapOpenStop },
	//};

	//SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gTrapAnimations[Animations::TRAP_IDLE], nullptr, 0, true);
	//SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gTrapAnimations[Animations::TRAP_OPEN], openOptions, 1, false);
	//mAnimationController->SetState(ANIM_STATE_IDLE);
	//mAnimationController->Resume();
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

void Trap::SpawnPoison(int UUID, vec3f position, float duration)
{
	Spawn(UUID, position, duration);
	mEffect->mOnUpdateCallback = StatusEffect::OnPoisonUpdate;
	mEffect->mDuration = duration;
}

void Trap::SpawnSlow(int UUID, vec3f position, float duration)
{
	Spawn(UUID, position, duration);
	mEffect->mOnUpdateCallback = StatusEffect::OnSlowUpdate;
	mEffect->mDuration = duration;
}

void Trap::Update(float seconds)
{
	// Check delay
	if (mDelay > 0.0f)
	{
		mDelay -= seconds;
		return;
	}

	// Make sure to activate component
	if (!mSphereColliderComponent->mIsActive)
	{
		mSphereColliderComponent->mIsActive = true;
	}

	if (mEffect->mIsActive)
	{
		// Update duration
		mDuration -= seconds;
		if (mDuration <= 0.0f)
		{
			mShouldDestroy = true;
		}
	}
}

void Trap::OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other)
{
	Trap* pTrap = reinterpret_cast<Trap*>(self);
	if (!pTrap->mEffect->mIsActive)
	{
		pTrap->mEffect->mIsActive = true;
		//pTrap->mAnimationController->SetState(ANIM_STATE_MELEE);
		//pTrap->mAnimationController->Resume();
	}
}

void Trap::OnTriggerStay(BaseSceneObject* self, BaseSceneObject* other)
{
	Trap* pTrap = reinterpret_cast<Trap*>(self);
	if (!pTrap->mEffect->mIsActive)
	{
		return;
	}

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

void Trap::OnTrapOpenStart(void* obj)
{
	
}

void Trap::OnTrapOpenStop(void* obj)
{
	Trap* pTrap = reinterpret_cast<Trap*>(obj);
	pTrap->mEffect->mIsActive = true;
}
