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

Minion::Minion()
{
	mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mIsDynamic = true;
	mCollider->mLayer = COLLISION_LAYER_MINION;

	mMeleeColliderComponent = Factory<SphereColliderComponent>::Create();
	mMeleeColliderComponent->mCollider.radius = 2.5f;
	mMeleeColliderComponent->mOffset = { 0.0f, 0.0f, 2.75f };
	mMeleeColliderComponent->mIsActive = false;
	mMeleeColliderComponent->mIsDynamic = true;
	mMeleeColliderComponent->mIsTrigger = true;
	mMeleeColliderComponent->mSceneObject = this;
	mMeleeColliderComponent->mLayer = COLLISION_LAYER_MINION_SKILL;
	mMeleeColliderComponent->RegisterTriggerEnterCallback(&MinionController::OnMeleeHit);

	mController = Factory<MinionController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->RegisterMoveCallback(&OnMove);
	
	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);

	//mAnimationController->RegisterCommandExecutedCallback(&OnAnimationCommandExecuted);

	//Animation melee = gMinionAnimations[Animations::MINION_ATTACK];
	//KeyframeOption meleeOptions[] = { { melee.startFrameIndex, OnMeleeStart },{ melee.endFrameIndex, OnMeleeStop } };
	//SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], meleeOptions, 2, false);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], nullptr, 0, false);
	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_WALK, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_RUN, &gMinionAnimations[Animations::MINION_RUN], nullptr, 0, true);
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);
}

Minion::~Minion()
{
}

void Minion::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;
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

		//m->mHealth->TakeDamage(1.0f);
		//if (m->mHealth->GetHealth() <= 0) e->mHealth->TakeDamage(-1000.0f);
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