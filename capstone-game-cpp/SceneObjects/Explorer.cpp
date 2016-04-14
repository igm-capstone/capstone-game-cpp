#include "stdafx.h"
#include "Explorer.h"
#include <Components/NetworkID.h>
#include <Components/ExplorerController.h>
#include <Components/ColliderComponent.h>
#include <ModelManager.h>
#include <Components/Health.h>
#include <Components/Skill.h>
#include <Components/AnimationUtility.h>

#define MELEE_SKILL_INDEX	0
#define SPRINT_SKILL_INDEX	1
#define HEAL_SKILL_INDEX	1
#define POISON_SKILL_INDEX  1
#define SLOW_SKILL_INDEX	2
#include "Minion.h"

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
	mNetworkID->RegisterNetSyncAnimationCallback(&OnNetSyncAnimation);

	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;
	mAnimationController->RegisterCommandExecutedCallback(&OnAnimationCommandExecuted);

	Animation melee = gMinionAnimations[Animations::MINION_ATTACK];
	KeyframeOption meleeOptions[] = { { melee.startFrameIndex, OnMeleeStart }, { melee.endFrameIndex, OnMeleeStop } };
	SetStateAnimation(mAnimationController, ANIM_STATE_WALK,  &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_RUN,   &gMinionAnimations[Animations::MINION_RUN], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], meleeOptions, 2, false);
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);

	//Animation melee = gSprinterAnimations[Animations::SPRINTER_ATTACK];
	//KeyframeOption meleeOptions[] = { { melee.startFrameIndex, OnMeleeStart },{ melee.endFrameIndex, OnMeleeStop } };
	//SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gSprinterAnimations[Animations::SPRINTER_IDLE], nullptr, 0, true);
	//SetStateAnimation(mAnimationController, ANIM_STATE_RUN, &gSprinterAnimations[Animations::SPRINTER_RUN], nullptr, 0, true);
	//SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gSprinterAnimations[Animations::SPRINTER_ATTACK], meleeOptions, 2, false);

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
	}

	auto& ai = Singleton<AIManager>::SharedInstance();
	Node* node = ai.GetNodeAt(newPos);

	if (node != e->mCurrentNode)
	{
		e->mCurrentNode = node;
		ai.SetGridDirty(true);
	}
}

void Explorer::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto e = static_cast<Explorer*>(obj);
	e->mController->mIsActive = newAuth;
	e->mCameraManager->MoveCamera(e->mTransform->GetPosition(), e->mTransform->GetPosition() + vec3f(0,-7,-15));

	if (!e->mNetworkID->mHasAuthority) 
	{
		return;
	}

	// Giving all explorers same melee collider for now.
	e->mMeleeColliderComponent.asSphereColliderComponent = Factory<SphereColliderComponent>::Create();
	e->mMeleeColliderComponent.asSphereColliderComponent->mCollider.radius = 2.0f;
	e->mMeleeColliderComponent.asSphereColliderComponent->mOffset = { 0.0f, 0.0f, 2.75f };
	e->mMeleeColliderComponent.asSphereColliderComponent->mIsActive = false;
	e->mMeleeColliderComponent.asSphereColliderComponent->mIsTrigger = true;
	e->mMeleeColliderComponent.asSphereColliderComponent->mIsDynamic = false;
	e->mMeleeColliderComponent.asSphereColliderComponent->mLayer = COLLISION_LAYER_EXPLORER_SKILL;
	e->mMeleeColliderComponent.asSphereColliderComponent->RegisterTriggerEnterCallback(&OnMeleeHit);
	e->mMeleeColliderComponent.asBaseColliderComponent->mSceneObject = e;

	// Add more as we get more classes.
	switch (GetExplorerID(e))
	{
	case 0:
	{
		auto heal = Factory<Skill>::Create();
		heal->SetBinding(SkillBinding().Set(KEYCODE_A));
		heal->Setup("Heal", 2, 1, DoHeal);
		heal->mSceneObject = e;
		e->mSkills[HEAL_SKILL_INDEX] = heal;

		auto melee = Factory<Skill>::Create();
		melee->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		melee->Setup("Melee", 2, 1, DoMelee);
		melee->mSceneObject = e;
		e->mSkills[MELEE_SKILL_INDEX] = melee;

		SkillBar* mSkillBar = &Application::SharedInstance().GetCurrentScene()->mSkillBar;
		mSkillBar->AddSkill(e->mSkills[MELEE_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 5, 8);
		mSkillBar->AddSkill(e->mSkills[HEAL_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 0, 6);

		break;
	}
	case 1:
	{
		auto poison = Factory<Skill>::Create();
		poison->SetBinding(SkillBinding().Set(KEYCODE_A));
		poison->Setup("Poison Trap", 2, 1, DoPoison);
		poison->mSceneObject = e;
		e->mSkills[POISON_SKILL_INDEX] = poison;

		auto slow = Factory<Skill>::Create();
		slow->SetBinding(SkillBinding().Set(KEYCODE_D));
		slow->Setup("Glue Trap", 2, 1, DoSlow);
		slow->mSceneObject = e;
		e->mSkills[SLOW_SKILL_INDEX] = slow;

		auto melee = Factory<Skill>::Create();
		melee->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		melee->Setup("Melee", 2, 1, DoMelee);
		melee->mSceneObject = e;
		e->mSkills[MELEE_SKILL_INDEX] = melee;

		SkillBar* mSkillBar = &Application::SharedInstance().GetCurrentScene()->mSkillBar;
		mSkillBar->AddSkill(e->mSkills[MELEE_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 5, 8);
		mSkillBar->AddSkill(e->mSkills[POISON_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 2, 6);
		mSkillBar->AddSkill(e->mSkills[SLOW_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 4, 4);

		break;
	}
	case 2:
	default:
	{
		auto sprint = Factory<Skill>::Create();
		sprint->SetBinding(SkillBinding().Set(KEYCODE_A));
		sprint->Setup("Sprint", 2, 1, DoSprint);
		sprint->mSceneObject = e;
		e->mSkills[SPRINT_SKILL_INDEX] = sprint;

		auto melee = Factory<Skill>::Create();
		melee->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		melee->Setup("Melee", 2, 1, DoMelee);
		melee->mSceneObject = e;
		e->mSkills[MELEE_SKILL_INDEX] = melee;

		SkillBar* mSkillBar = &Application::SharedInstance().GetCurrentScene()->mSkillBar;
		mSkillBar->AddSkill(e->mSkills[MELEE_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 5, 8);
		mSkillBar->AddSkill(e->mSkills[SPRINT_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 1, 6);

		break;
	}
	}
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

void Explorer::OnAnimationCommandExecuted(BaseSceneObject* obj, AnimationControllerState state, AnimationControllerCommand command) {
	auto e = static_cast<Explorer*>(obj);

	if (e->mNetworkID->mHasAuthority) {
		Packet p(PacketTypes::SYNC_ANIMATION);
		p.UUID = e->mNetworkID->mUUID;
		p.AsAnimation.State = state;
		p.AsAnimation.Command = command;
		e->mNetworkClient->SendData(&p);
	}
}

void Explorer::OnNetSyncAnimation(BaseSceneObject* obj, byte state, byte command)
{
	auto e = static_cast<Explorer*>(obj);
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

void Explorer::OnHealthChange(BaseSceneObject* obj, float newVal, bool checkAuthority)
{
	auto e = static_cast<Explorer*>(obj);
	if (!checkAuthority || e->mNetworkID->mHasAuthority)
	{
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

bool Explorer::DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->Sprint(duration);
	return true;
}

bool Explorer::DoMelee(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->Melee();
	return true;
}

bool Explorer::DoHeal(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	Explorer* explorer = reinterpret_cast<Explorer*>(obj);
	if (explorer->mNetworkID->mHasAuthority)
	{
		Packet p(PacketTypes::SPAWN_SKILL);
		p.AsSkill.Position = explorer->mTransform->GetPosition();
		p.AsSkill.Duration = explorer->mSkills[HEAL_SKILL_INDEX]->mDuration;
		p.AsSkill.Type = SkillPacketTypes::SKILL_TYPE_HEAL;
		p.UUID = explorer->mNetworkID->mUUID;
		explorer->mNetworkClient->SendData(&p);
	}
	return true;
}

bool Explorer::DoPoison(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	Explorer* explorer = reinterpret_cast<Explorer*>(obj);
	if (explorer->mNetworkID->mHasAuthority)
	{
		Packet p(PacketTypes::SPAWN_SKILL);
		p.AsSkill.Position = explorer->mTransform->GetPosition();
		p.AsSkill.Duration = explorer->mSkills[POISON_SKILL_INDEX]->mDuration;
		p.AsSkill.Type = SkillPacketTypes::SKILL_TYPE_POISON;
		p.UUID = explorer->mNetworkID->mUUID;
		explorer->mNetworkClient->SendData(&p);
	}
	return true;
}

bool Explorer::DoSlow(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	Explorer* explorer = reinterpret_cast<Explorer*>(obj);
	if (explorer->mNetworkID->mHasAuthority)
	{
		Packet p(PacketTypes::SPAWN_SKILL);
		p.AsSkill.Position = explorer->mTransform->GetPosition();
		p.AsSkill.Duration = explorer->mSkills[SLOW_SKILL_INDEX]->mDuration;
		p.AsSkill.Type = SkillPacketTypes::SKILL_TYPE_SLOW;
		p.UUID = explorer->mNetworkID->mUUID;
		explorer->mNetworkClient->SendData(&p);
	}
	return true;
}

void Explorer::OnMeleeStart(void* obj)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	if (e->mNetworkID->mHasAuthority)
	{
		e->mMeleeColliderComponent.asBaseColliderComponent->mIsActive = true;
	}
}

void Explorer::OnMeleeStop(void* obj)
{
	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->PlayStateAnimation(ANIM_STATE_IDLE);
	
	if (e->mNetworkID->mHasAuthority)
	{
		e->mMeleeColliderComponent.asBaseColliderComponent->mIsActive = false;
	}
}

void Explorer::OnMeleeHit(BaseSceneObject* self, BaseSceneObject* other)
{	
	// THis is currently an assumption that this object will have a Health component
	if (other->Is<Minion>())
	{
		auto m = reinterpret_cast<Minion*>(other);
		m->mHealth->TakeDamage(100.0f);
	}
	else if (other->Is<Explorer>())
	{
		// Check Friendly Fire
		auto e = reinterpret_cast<Explorer*>(other);
		e->mHealth->TakeDamage(100.0f, false);
	}
}