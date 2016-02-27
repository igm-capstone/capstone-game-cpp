#include "stdafx.h"
#include "Explorer.h"
#include <Components/NetworkID.h>
#include <Components/ExplorerController.h>
#include <Components/AnimationController.h>
#include <Components/ColliderComponent.h>
#include <ModelManager.h>
#include <Components/Health.h>
#include <Components/Skill.h>
#include <Vertex.h>
#include <Components/AnimationUtility.h>

#define SPRINT_SKILL_INDEX	0
#define MELEE_SKILL_INDEX	1

int GetExplorerID(Explorer* explorer)
{
	return explorer->mNetworkID->mUUID - 1;
}

Explorer::Explorer()
{
	mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;
	mCameraManager = &Singleton<CameraManager>::SharedInstance();

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
	mNetworkID->RegisterNetSyncTransformCallback(&OnNetSyncTransform);
	mNetworkID->RegisterNetHealthChangeCallback(&OnNetHealthChange);

	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	KeyframeOption meleeOptions[] = { { gMinionMelee.startFrameIndex, OnMeleeStart }, { gMinionMelee.endFrameIndex, OnMeleeStop } };
	SetStateAnimation(mAnimationController, ANIM_STATE_WALK, &gMinionWalk, nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_RUN, &gMinionRun, nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionMelee, meleeOptions, 2, false);
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);

	mController = Factory<ExplorerController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->mSpeed = 0.05f;
	mController->RegisterMoveCallback(&OnMove);
	mController->mAnimationController = mAnimationController;	// Be careful if you move this code. AnimationController should exist before here.

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mIsDynamic = true;
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mLayer = COLLISION_LAYER_EXPLORER;
	mCollider->RegisterCollisionExitCallback(&OnCollisionExit);

	mHealth = Factory<Health>::Create();
	mHealth->mSceneObject = this;
	mHealth->SetMaxHealth(1000.0f);
	mHealth->RegisterHealthChangeCallback(OnHealthChange);
}


void Explorer::DebugSpawn(vec3f pos, int UUID)
{
	Spawn(pos, UUID);
	mNetworkID->mHasAuthority = true;
	mController->mIsActive = true;
}

void Explorer::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);		

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	mAnimationController->SetState(ANIM_STATE_IDLE);

	// Add more as we get more classes.
	switch (GetExplorerID(this))
	{
	case 0:
	default:
	{
		mMeleeColliderComponent.asSphereColliderComponent = Factory<SphereColliderComponent>::Create();
		mMeleeColliderComponent.asSphereColliderComponent->mCollider.radius = 2.5f;
		mMeleeColliderComponent.asSphereColliderComponent->mOffset = { 0.0f, 0.0f, 2.75f };
		mMeleeColliderComponent.asSphereColliderComponent->mIsActive = false;
		mMeleeColliderComponent.asSphereColliderComponent->mIsTrigger = true;
		mMeleeColliderComponent.asSphereColliderComponent->mIsDynamic = false;
		mMeleeColliderComponent.asSphereColliderComponent->mLayer = COLLISION_LAYER_SKILL;
		mMeleeColliderComponent.asSphereColliderComponent->RegisterTriggerEnterCallback(&OnMeleeHit);

		auto sprint = Factory<Skill>::Create();
		sprint->SetBinding(SkillBinding().Set(KEYCODE_A));
		sprint->Setup(2, 1, DoSprint);
		sprint->mSceneObject = this;
		mSkills[SPRINT_SKILL_INDEX] = sprint;

		auto melee = Factory<Skill>::Create();
		melee->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		melee->Setup(2, 1, DoMelee);
		melee->mSceneObject = this;
		mSkills[MELEE_SKILL_INDEX] = melee;

		break;
	}
	}
}

void Explorer::OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->UpdateComponents(newRot, newPos);

	if (e->mNetworkID->mHasAuthority) {
		e->mCameraManager->ChangeLookAtTo(newPos);
		Packet p(PacketTypes::SYNC_TRANSFORM);
		p.UUID = e->mNetworkID->mUUID;
		p.AsTransform.Position = newPos;
		p.AsTransform.Rotation = newRot;
		e->mNetworkClient->SendData(&p);

		e->mHealth->TakeDamage(1.0f);
		if (e->mHealth->GetHealth() <= 0) e->mHealth->TakeDamage(-1000.0f);
	}

	Singleton<AIManager>::SharedInstance().SetGridDirty(true);
}

void Explorer::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto e = static_cast<Explorer*>(obj);
	e->mController->mIsActive = newAuth;
	e->mCameraManager->MoveCamera(e->mTransform->GetPosition(), e->mTransform->GetPosition() + vec3f(0,-7,-15));
}

void Explorer::OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->UpdateComponents(newRot, newPos);
}

void Explorer::OnNetHealthChange(BaseSceneObject* obj, float newVal)
{
	auto e = static_cast<Explorer*>(obj);
	e->mHealth->SetHealth(newVal);
}

void Explorer::OnHealthChange(BaseSceneObject* obj, float newVal)
{
	auto e = static_cast<Explorer*>(obj);
	if (e->mNetworkID->mHasAuthority) {
		Packet p(PacketTypes::SYNC_HEALTH);
		p.UUID = e->mNetworkID->mUUID;
		p.AsFloat = newVal;
		e->mNetworkClient->SendData(&p);
	}
}

void Explorer::OnCollisionExit(BaseSceneObject* obj, BaseSceneObject* other)
{
	other->Is<Explorer>();
	auto e = static_cast<Explorer*>(obj);
	if (e->mNetworkID->mHasAuthority) {
		e->mCameraManager->ChangeLookAtTo(e->mTransform->GetPosition());
	}
}

void Explorer::UpdateComponents(quatf rotation, vec3f position)
{
	mCollider->mCollider.origin = position;
	
	// Add more cases as we add explorer types
	switch (GetExplorerID(this))
	{
	case 0:
	default:
	{
		if (mMeleeColliderComponent.asBaseColliderComponent)
		{
			mMeleeColliderComponent.asSphereColliderComponent->mCollider.origin = position + (rotation * mMeleeColliderComponent.asSphereColliderComponent->mOffset);
		}
		break;
	}
	}
}

void Explorer::DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->Sprint(duration);
}

void Explorer::DoMelee(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->Melee();
}

void Explorer::OnMeleeStart(void* obj)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mMeleeColliderComponent.asBaseColliderComponent->mIsActive = true;
}

void Explorer::OnMeleeStop(void* obj)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mMeleeColliderComponent.asBaseColliderComponent->mIsActive = false;
	e->mController->PlayStateAnimation(ANIM_STATE_IDLE);
}

void Explorer::OnMeleeHit(BaseSceneObject* self, BaseSceneObject* other)
{	
	// THis is currently an assumption that this object will have a Health component
	auto e = reinterpret_cast<Explorer*>(other);
	e->mHealth->TakeDamage(100.0f);
}