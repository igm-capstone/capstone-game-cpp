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

struct ExplorerInventory
{
	BaseColliderComponent* meleeCollider;
	Skill* skills[MAX_EXPLORER_SKILLS];

} gExplorerInventory[MAX_EXPLORERS];

void InitializeExplorerInventory()
{
	gExplorerInventory[0].meleeCollider = Factory<OrientedBoxColliderComponent>::Create();
	gExplorerInventory[1].meleeCollider = Factory<SphereColliderComponent>::Create();
	gExplorerInventory[2].meleeCollider = Factory<SphereColliderComponent>::Create();
	gExplorerInventory[3].meleeCollider = Factory<SphereColliderComponent>::Create();

	for (int i = 0; i < MAX_EXPLORERS; i++)
	{
		gExplorerInventory[i].meleeCollider->mIsActive	= false;
		gExplorerInventory[i].meleeCollider->mIsTrigger = true;
		gExplorerInventory[i].meleeCollider->mIsDynamic = false;
		gExplorerInventory[i].meleeCollider->mLayer		= COLLISION_LAYER_SKILL;

		auto sprint = Factory<Skill>::Create();
		sprint->SetBinding(SkillBinding().Set(KEYCODE_A));
		sprint->Setup(2, 1, Explorer::DoSprint);
		sprint->mIsActive = false;
		gExplorerInventory[i].skills[0] = sprint;

		auto melee = Factory<Skill>::Create();
		melee->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		melee->Setup(2, 1, Explorer::DoMelee);
		melee->mIsActive = false;
		gExplorerInventory[i].skills[1] = melee;
	}
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

	Application::SharedInstance().GetModelManager()->LoadModel<GPU::SkinnedVertex>("Minion_Test");
	Application::SharedInstance().GetModelManager()->GetModel("Minion_Test")->Link(this);

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
	mController->SetBaseRotation(PI * 0.5, PI, 0.0f);
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

void Explorer::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);		

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	mMeleeCollider	= gExplorerInventory[UUID].meleeCollider;
	mMeleeCollider->mSceneObject = this;

	mSkills			= &gExplorerInventory[UUID].skills[0];
	for (int i = 0; i < MAX_EXPLORER_SKILLS; i++)
	{
		mSkills[i]->mSceneObject = this;
		mSkills[i]->mIsActive = true;
	}

	mAnimationController->SetState(ANIM_STATE_IDLE);
}

void Explorer::OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->mCollider->mCollider.origin = newPos;
		
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
	e->mCollider->mCollider.origin = newPos;
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
	auto e = static_cast<Explorer*>(obj);
	if (e->mNetworkID->mHasAuthority) {
		e->mCameraManager->ChangeLookAtTo(e->mTransform->GetPosition());
	}
}

void Explorer::DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	TRACE_LOG("Sprint!!");

	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->Sprint(duration);
}

void Explorer::DoMelee(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	TRACE_LOG("DoMelee " << obj);
	AnimationController* pAnimationController = reinterpret_cast<Explorer*>(obj)->mAnimationController;
	pAnimationController->SetState(ANIM_STATE_MELEE);
	pAnimationController->Resume();
}

void Explorer::OnMeleeStart(void* obj)
{
	TRACE_LOG("ANIM START");
}

void Explorer::OnMeleeStop(void* obj)
{
	TRACE_LOG("ANIM STOP");

	AnimationController* pAnimationController = reinterpret_cast<Explorer*>(obj)->mAnimationController;
	pAnimationController->SetState(ANIM_STATE_WALK);
	pAnimationController->Resume();
}