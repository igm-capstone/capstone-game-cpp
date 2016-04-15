#include "stdafx.h"
#include "Minion.h"
#include <Components/NetworkID.h>
#include <Components/ColliderComponent.h>
#include <Components/MinionController.h>
#include <Components/AnimationController.h>
#include <ScareTacticsApplication.h>
#include <Components/AnimationUtility.h>
#include <Components/Health.h>
#include <Network\NetworkClient.h>
#include <Components/ImpController.h>
#include <Components/FlyTrapController.h>

Minion::Minion()
	: mClass(UNKNOWN)
	, mNetworkClient(&Singleton<NetworkManager>::SharedInstance().mClient)
{
	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
	mNetworkID->RegisterNetHealthChangeCallback(&OnNetHealthChange);

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mIsDynamic = true;
	mCollider->mLayer = COLLISION_LAYER_MINION;

	mHealth = Factory<Health>::Create();
	mHealth->mSceneObject = this;
	mHealth->SetMaxHealth(1000.0f);
	mHealth->RegisterHealthChangeCallback(OnHealthChange);
}

Minion::~Minion()
{
	Factory<NetworkID>::Destroy(mNetworkID);
	Factory<SphereColliderComponent>::Destroy(mCollider);
	Factory<SphereColliderComponent>::Destroy(mMeleeColliderComponent);

	Factory<Health>::Destroy(mHealth);
	Factory<AnimationController>::Destroy(mAnimationController);

	switch(mClass)
	{
	case IMP:
		Factory<ImpController>::Destroy(reinterpret_cast<ImpController*>(mController));
		break;
	case FLYTRAP:
		Factory<FlyTrapController>::Destroy(reinterpret_cast<FlyTrapController*>(mController));
		break;
	case ABOMINATION:
		break;
	default:
		break;
	}
}

void Minion::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);
	mTransform->SetRotation(mController->GetAdjustedRotation(0));

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	mAnimationController->SetState(ANIM_STATE_IDLE);
	mAnimationController->Resume();
}

void Minion::SpawnImp(vec3f pos, int UUID)
{
	mClass = IMP;

	mMeleeColliderComponent = Factory<SphereColliderComponent>::Create();
	mMeleeColliderComponent->mSceneObject = this;
	mMeleeColliderComponent->mCollider.radius = 2.5f;
	mMeleeColliderComponent->mOffset = { 0.0f, 0.0f, 2.75f };
	mMeleeColliderComponent->mIsActive = false;
	mMeleeColliderComponent->mIsDynamic = true;
	mMeleeColliderComponent->mIsTrigger = true;
	mMeleeColliderComponent->mLayer = COLLISION_LAYER_MINION_SKILL;
	mMeleeColliderComponent->RegisterTriggerEnterCallback(&MinionController::OnMeleeHit);

	mController = Factory<ImpController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->RegisterMoveCallback(&OnMove);

	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	Animation melee = gMinionAnimations[Animations::MINION_ATTACK];
	KeyframeOption meleeOptions[] = {
		{ melee.startFrameIndex + 10, &MinionController::OnMeleeStart },
		{ melee.endFrameIndex,        &MinionController::OnMeleeStop },
	};

	//SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], nullptr, 0, false);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], meleeOptions, 2, false);
	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_WALK, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_RUN, &gMinionAnimations[Animations::MINION_RUN], nullptr, 0, true);
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);

	Spawn(pos, UUID);
}


void Minion::SpawnFlytrap(vec3f pos, int UUID)
{
	mClass = FLYTRAP;

	mMeleeColliderComponent = Factory<SphereColliderComponent>::Create();
	mMeleeColliderComponent->mSceneObject = this;
	mMeleeColliderComponent->mCollider.radius = 2.5f;
	mMeleeColliderComponent->mOffset = { 0.0f, 0.0f, 2.75f };
	mMeleeColliderComponent->mIsActive = false;
	mMeleeColliderComponent->mIsDynamic = false;
	mMeleeColliderComponent->mIsTrigger = true;
	mMeleeColliderComponent->mLayer = COLLISION_LAYER_MINION_SKILL;
	mMeleeColliderComponent->RegisterTriggerEnterCallback(&MinionController::OnMeleeHit);

	mController = Factory<FlyTrapController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->RegisterMoveCallback(&OnMove);

	Application::SharedInstance().GetModelManager()->GetModel(kPlantModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	Animation melee = gPlantAnimations[Animations::PLANT_BITE];
	KeyframeOption meleeOptions[] = {
		{ melee.startFrameIndex + 10, &MinionController::OnMeleeStart },
		{ melee.endFrameIndex,        &MinionController::OnMeleeStop },
	};

	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gPlantAnimations[Animations::PLANT_IDLE], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gPlantAnimations[Animations::PLANT_BITE], meleeOptions, 2, false);

	Spawn(pos, UUID);
}


void Minion::OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto m = static_cast<Minion*>(obj);
	m->mTransform->SetPosition(newPos);
	m->mTransform->SetRotation(newRot);

	m->UpdateComponents(newRot, newPos);

	if (m->mNetworkID->mHasAuthority) {
		Packet p(PacketTypes::SYNC_TRANSFORM);
		p.UUID = m->mNetworkID->mUUID;
		p.AsTransform.Position = newPos;
		p.AsTransform.Rotation = newRot;
		m->mNetworkClient->SendData(&p);
	}
}


void Minion::UpdateComponents(quatf rotation, vec3f position)
{
	mCollider->mCollider.origin = position;
	mMeleeColliderComponent->mCollider.origin = position + (rotation * mMeleeColliderComponent->mOffset);
}

void Minion::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto m = static_cast<Minion*>(obj);
	m->mController->mIsActive = newAuth;
}

void Minion::OnHealthChange(BaseSceneObject* obj, float newVal, bool shouldCheckAuthority)
{
	Minion* m = reinterpret_cast<Minion*>(obj);
	if (!shouldCheckAuthority || m->mNetworkID->mHasAuthority)
	{
		Packet p(PacketTypes::SYNC_HEALTH);
		p.UUID = m->mNetworkID->mUUID;
		p.AsFloat = newVal;
		m->mNetworkClient->SendData(&p);	
	}

	if (m->mHealth->GetHealth() <= 0.0f)
	{
		m->mShouldDestroy = true;
	}
}

void Minion::OnNetHealthChange(BaseSceneObject* obj, float newVal)
{
	auto e = static_cast<Minion*>(obj);
	e->mHealth->SetHealth(newVal);
}