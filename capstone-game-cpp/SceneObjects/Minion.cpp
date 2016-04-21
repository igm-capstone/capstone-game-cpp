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
	, mNetworkServer(&Singleton<NetworkManager>::SharedInstance().mServer)
{
	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
	mNetworkID->RegisterNetSyncTransformCallback(&OnNetSyncTransform);
	mNetworkID->RegisterNetHealthChangeCallback(&OnNetHealthChange);
	mNetworkID->RegisterNetSyncAnimationCallback(&OnNetSyncAnimation);

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mIsDynamic = true;
	mCollider->mLayer = COLLISION_LAYER_MINION;

	mMeleeColliderComponent = Factory<SphereColliderComponent>::Create();
	mMeleeColliderComponent->mSceneObject = this;
	mMeleeColliderComponent->mIsActive = false;
	mMeleeColliderComponent->mIsDynamic = true;
	mMeleeColliderComponent->mIsTrigger = true;
	mMeleeColliderComponent->mLayer = COLLISION_LAYER_MINION_SKILL;
	mMeleeColliderComponent->RegisterTriggerEnterCallback(&MinionController::OnMeleeHit);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->RegisterCommandExecutedCallback(&OnAnimationCommandExecuted);

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

	mController->mSceneObject = this;
	mController->RegisterMoveCallback(&OnMove);

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;
}

void Minion::SpawnImp(vec3f pos, int UUID)
{
	mClass = IMP;
	mController = Factory<ImpController>::Create();
	Spawn(pos, UUID);

	mTransform->SetScale(0.3f);
	mCollider->mOffset = { 0.0f, 0.65f, 0.0f };
	mCollider->mCollider.radius = 1.45f;

	mMeleeColliderComponent->mCollider.radius = 1.0f;
	mMeleeColliderComponent->mOffset = { 0.0f, 0.65f, mCollider->mCollider.radius + mMeleeColliderComponent->mCollider.radius + 0.01f };

	mController->mSceneObject = this;

	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	Animation melee = gMinionAnimations[Animations::MINION_ATTACK];
	KeyframeOption meleeOptions[] = {
		{ melee.startFrameIndex + 10, &MinionController::OnMeleeStart },
		{ melee.endFrameIndex,        &MinionController::OnMeleeStop },
	};

	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], meleeOptions, 2, false);
	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_WALK, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_RUN, &gMinionAnimations[Animations::MINION_RUN], nullptr, 0, true);
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);
}


void Minion::SpawnFlytrap(vec3f pos, int UUID)
{
	mClass = FLYTRAP;
	mController = Factory<FlyTrapController>::Create();
	Spawn(pos, UUID);

	mTransform->SetScale(0.35f);
	mCollider->mOffset = { 0.0f, 0.75f, 0.0f };
	mCollider->mCollider.radius = 2.0f;
	mCollider->mIsDynamic = false;

	mMeleeColliderComponent->mCollider.radius = 1.0f;
	mMeleeColliderComponent->mOffset = { 0.0f, 0.65f, mCollider->mCollider.radius + mMeleeColliderComponent->mCollider.radius + 0.01f };
	mMeleeColliderComponent->mIsDynamic = false;

	Application::SharedInstance().GetModelManager()->GetModel(kPlantModelName)->Link(this);
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	Animation melee = gPlantAnimations[Animations::PLANT_BITE];
	KeyframeOption meleeOptions[] = {
		{ melee.startFrameIndex + 10, &MinionController::OnMeleeStart },
		{ melee.endFrameIndex,        &MinionController::OnMeleeStop },
	};

	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gPlantAnimations[Animations::PLANT_IDLE], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gPlantAnimations[Animations::PLANT_BITE], meleeOptions, 2, false);
	mAnimationController->SetState(ANIM_STATE_IDLE);
	mAnimationController->Resume();
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
		m->mNetworkServer->SendToAll(&p);
	}
}


void Minion::UpdateComponents(quatf rotation, vec3f position)
{
	mCollider->mCollider.origin = position + (rotation * mCollider->mOffset);
	mMeleeColliderComponent->mCollider.origin = position + (rotation * mMeleeColliderComponent->mOffset);
}

void Minion::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto m = static_cast<Minion*>(obj);
	m->mController->mIsActive = newAuth;
}


void Minion::OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Minion*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->UpdateComponents(newRot, newPos);
}

void Minion::OnHealthChange(BaseSceneObject* obj, float newVal, bool shouldCheckAuthority)
{
	Minion* m = reinterpret_cast<Minion*>(obj);
	if (!shouldCheckAuthority || m->mNetworkID->mHasAuthority)
	{
		Packet p(PacketTypes::SYNC_HEALTH);
		p.UUID = m->mNetworkID->mUUID;
		p.AsFloat = newVal;

		if (Singleton<NetworkManager>::SharedInstance().mMode == NetworkManager::CLIENT)
		{
			Singleton<NetworkManager>::SharedInstance().mClient.SendData(&p);
		}
		else
		{
			m->mNetworkServer->SendToAll(&p);
		}

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

void Minion::OnAnimationCommandExecuted(BaseSceneObject* obj, AnimationControllerState state, AnimationControllerCommand command) {
	auto e = static_cast<Minion*>(obj);

	if (e->mNetworkID->mHasAuthority) {
		Packet p(PacketTypes::SYNC_ANIMATION);
		p.UUID = e->mNetworkID->mUUID;
		p.AsAnimation.State = state;
		p.AsAnimation.Command = command;
		e->mNetworkServer->SendToAll(&p);
	}
}

void Minion::OnNetSyncAnimation(BaseSceneObject* obj, byte state, byte command)
{
	auto e = static_cast<Minion*>(obj);
	switch (command)
	{
	case ANIM_STATE_COMMAND_PLAY:
		e->mController->PlayStateAnimation(AnimationControllerState(state));
		break;
	case ANIM_STATE_COMMAND_PAUSE:
		e->mController->PauseStateAnimation(AnimationControllerState(state));
		break;
	}
}