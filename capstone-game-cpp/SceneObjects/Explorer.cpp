#include "stdafx.h"
#include "Explorer.h"
#include <Components/NetworkID.h>
#include <Components/ExplorerController.h>
#include <Components/ColliderComponent.h>
#include <ModelManager.h>
#include <Components/Health.h>
#include <Components/Skill.h>
#include <Components/AnimationUtility.h>
#include "jsonUtils.h"
#include "Minion.h"

#define MELEE_SKILL_INDEX	0
#define SPRINT_SKILL_INDEX	1
#define HEAL_SKILL_INDEX	1
#define POISON_SKILL_INDEX  1
#define SLOW_SKILL_INDEX	2
#define LANTERN_SKILL_INDEX 3

using namespace nlohmann;
using jarr_t = json::array_t;

Skill* createExplorerSkill(Skill::UseCallback callback, SkillBinding binding, json& skillConfig)
{
	auto cooldown = skillConfig["cooldown"].get<float>();
	auto cost = skillConfig["cost"].get<float>();
	auto name = skillConfig["description"].get<string>();
	auto duration = skillConfig.find("duration") == skillConfig.end() ? 0.0f : skillConfig["duration"].get<float>();

	auto skill = Factory<Skill>::Create();
	skill->Setup(name, cooldown, duration, callback, cost);
	skill->SetBinding(binding);

	return skill;
}

Explorer::Explorer()
	: mAttackDamage(0)
	, mIsDead(false)
{
	mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;
	mCameraManager = &Singleton<CameraManager>::SharedInstance();

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
	mNetworkID->RegisterNetSyncTransformCallback(&OnNetSyncTransform);
	//mNetworkID->RegisterNetHealthChangeCallback(&OnNetHealthChange);
	mNetworkID->RegisterNetSyncAnimationCallback(&OnNetSyncAnimation);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->RegisterStateChangedCallback(&OnAnimStateChange);
	mAnimationController->RegisterCommandExecutedCallback(&OnAnimationCommandExecuted);

	mController = Factory<ExplorerController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->RegisterMoveCallback(&OnMove);
	mController->mAnimationController = mAnimationController;	// Be careful if you move this code. AnimationController should exist before here.

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mCollider.radius = 0.5f;
	mCollider->mIsDynamic = true;
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mLayer = COLLISION_LAYER_EXPLORER;
	mCollider->mOffset = { 0.0f, 1.0f, 0.0f };
	mCollider->RegisterCollisionExitCallback(&OnCollisionExit);
	
	mInteractionCollider = Factory<SphereColliderComponent>::Create();
	mInteractionCollider->mCollider.radius = 2.0f;
	mInteractionCollider->mIsDynamic = true;
	mInteractionCollider->mSceneObject = this;
	mInteractionCollider->mIsActive = false;
	mInteractionCollider->mIsTrigger = true;
	mInteractionCollider->mLayer = COLLISION_LAYER_EXPLORER_SKILL;
	mInteractionCollider->RegisterTriggerStayCallback(&OnTriggerStay);

	mHealth = Factory<Health>::Create();
	mHealth->mSceneObject = this;
	mHealth->SetupNetworkID(mNetworkID);
	mHealth->SetMaxHealth(1000.0f);
	mHealth->RegisterHealthChangeCallback(OnHealthChange);
	mHealth->RegisterHealthToZeroCallback(OnDeath);
	mHealth->RegisterHealthFromZeroCallback(OnRevive);
}
Explorer::~Explorer()
{
	Factory<NetworkID>::Destroy(mNetworkID);
	Factory<ExplorerController>::Destroy(mController);
	Factory<AnimationController>::Destroy(mAnimationController);
	Factory<SphereColliderComponent>::Destroy(mCollider);
	Factory<Health>::Destroy(mHealth);

	// Skills
	Factory<Skill>::Destroy(mSkills[MELEE_SKILL_INDEX]);
	Factory<Skill>::Destroy(mSkills[LANTERN_SKILL_INDEX]);

	switch (GetExplorerType())
	{
	case HEALER:
	{
		Factory<Skill>::Destroy(mSkills[HEAL_SKILL_INDEX]);
		break;
	}
	case TRAPMASTER:
	{
		Factory<Skill>::Destroy(mSkills[POISON_SKILL_INDEX]);
		Factory<Skill>::Destroy(mSkills[SLOW_SKILL_INDEX]);
		break;
	}
	case SPRINTER:
	default:
	{
		Factory<Skill>::Destroy(mSkills[SPRINT_SKILL_INDEX]);
		break;
	}
	}

	// This will change once explorers have unique melee attacks / colliders.
	Factory<SphereColliderComponent>::Destroy(mMeleeColliderComponent.asSphereColliderComponent);
}

void Explorer::DebugSpawn(vec3f pos, int UUID)
{
	Spawn(pos, UUID);
	mNetworkID->mHasAuthority = true;
	mController->mIsActive = true;
}

void Explorer::Spawn(vec3f pos, int UUID)
{
	mExplorerType = static_cast<ExplorerType>(UUID);

	mTransform->SetRotation(-0.5f * PI, 0, 0);
	mTransform->SetPosition(pos);		

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos + (mTransform->GetRotation() * mCollider->mOffset);

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	switch (GetExplorerType())
	{
	case HEALER:
		SetSprinterAnimations(this);
	//	SetProfessorAnimations(this);
		break;
	case SPRINTER:
		SetSprinterAnimations(this);
		break;
	case TRAPMASTER:
		SetTrapperAnimations(this);
		mTransform->SetScale(vec3f(0.5f));
		break;
	default:
		break;
	}

	mController->PlayStateAnimation(ANIM_STATE_IDLE);
}

void Explorer::OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot, bool sync)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->UpdateComponents(newRot, newPos);

	if (sync && e->mNetworkID->mHasAuthority) {
		e->mCameraManager->ChangeLookAtTo(newPos);
		Packet p(PacketTypes::SYNC_TRANSFORM);
		p.UUID = e->mNetworkID->mUUID;
		p.AsTransform.Position = newPos;
		p.AsTransform.Rotation = newRot;
		e->mNetworkClient->SendData(&p);
	}

	if (gDebugExplorer) {
		auto& ai = Singleton<AIManager>::SharedInstance();
		Node* node = ai.GetNodeAt(newPos);

		if (node != e->mCurrentNode)
		{
			e->mCurrentNode = node;
			ai.SetGridDirty(true);
		}
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
	e->mMeleeColliderComponent.asSphereColliderComponent->mCollider.radius = 1.0f;
	e->mMeleeColliderComponent.asSphereColliderComponent->mOffset = { 0.0f, 2.5f, e->mCollider->mCollider.radius + e->mMeleeColliderComponent.asSphereColliderComponent->mCollider.radius + 0.01f };
	e->mMeleeColliderComponent.asSphereColliderComponent->mIsActive = false;
	e->mMeleeColliderComponent.asSphereColliderComponent->mIsTrigger = true;
	e->mMeleeColliderComponent.asSphereColliderComponent->mIsDynamic = false;
	e->mMeleeColliderComponent.asSphereColliderComponent->mLayer = COLLISION_LAYER_EXPLORER_SKILL;
	e->mMeleeColliderComponent.asSphereColliderComponent->RegisterTriggerEnterCallback(&OnMeleeHit);
	e->mMeleeColliderComponent.asBaseColliderComponent->mSceneObject = e;

	jarr_t explorers = Application::SharedInstance().GetConfigJson()["explorers"].get<jarr_t>();

	json config;
	jarr_t skills;

	e->mSkills[LANTERN_SKILL_INDEX] = Factory<Skill>::Create();
	e->mSkills[LANTERN_SKILL_INDEX]->Setup("Lantern", 5, 5, DoLantern, 0);
	e->mSkills[LANTERN_SKILL_INDEX]->SetBinding(MOUSEBUTTON_RIGHT);
	e->mSkills[LANTERN_SKILL_INDEX]->mSceneObject = e;

	// Add more as we get more classes.
	switch (e->GetExplorerType())
	{
	case HEALER:
	{
		config = findByName(explorers, "LongAttackSupport");
		skills = config["skills"].get<jarr_t>();

		auto healConfig = findByName(skills, "Heal");
		e->mSkills[HEAL_SKILL_INDEX] = createExplorerSkill(DoHeal, KEYCODE_Q, healConfig);
		e->mSkills[HEAL_SKILL_INDEX]->mSceneObject = e;

		auto meleeConfig = findByName(skills, "LongAttack");
		e->mSkills[MELEE_SKILL_INDEX] = createExplorerSkill(DoMelee, MOUSEBUTTON_LEFT, meleeConfig);
		e->mSkills[MELEE_SKILL_INDEX]->mSceneObject = e;

		UIManager* mUIManager = &Application::SharedInstance().GetCurrentScene()->mUIManager;
		mUIManager->AddSkill(e->mSkills[MELEE_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 5, 8);
		mUIManager->AddSkill(e->mSkills[HEAL_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 0, 6);

		e->mAttackDamage = meleeConfig["damage"].get<float>();

		break;
	}
	case TRAPMASTER:
	{
		config = findByName(explorers, "GrenadeTrapper");
		skills = config["skills"].get<jarr_t>();

		auto poison = Factory<Skill>::Create();
		poison->SetBinding(SkillBinding().Set(KEYCODE_Q));
		poison->Setup("Poison Trap", 2.0f, 5.0f, DoPoison);
		poison->mSceneObject = e;
		e->mSkills[POISON_SKILL_INDEX] = poison;


		auto poisonConfig = findByName(skills, "SetTrapPoison");
		e->mSkills[POISON_SKILL_INDEX] = createExplorerSkill(DoPoison, KEYCODE_Q, poisonConfig);
		e->mSkills[POISON_SKILL_INDEX]->mSceneObject = e;

		auto glueConfig = findByName(skills, "SetTrapGlue");
		e->mSkills[SLOW_SKILL_INDEX] = createExplorerSkill(DoSlow, KEYCODE_E, glueConfig);
		e->mSkills[SLOW_SKILL_INDEX]->mSceneObject = e;

		auto tossConfig = findByName(skills, "GrenadeToss");
		e->mSkills[MELEE_SKILL_INDEX] = createExplorerSkill(DoMelee, MOUSEBUTTON_LEFT, tossConfig);
		e->mSkills[MELEE_SKILL_INDEX]->mSceneObject = e;

		// Hard coding until we 
		e->mSkills[POISON_SKILL_INDEX]->mDuration = 5.0f;
		e->mSkills[SLOW_SKILL_INDEX]->mDuration = 5.0f;

		UIManager* mUIManager = &Application::SharedInstance().GetCurrentScene()->mUIManager;
		mUIManager->AddSkill(e->mSkills[MELEE_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 5, 8);
		mUIManager->AddSkill(e->mSkills[POISON_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 2, 6);
		mUIManager->AddSkill(e->mSkills[SLOW_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 4, 4);

		e->mAttackDamage = tossConfig["damage"].get<float>();

		break;
	}
	case SPRINTER:
	default:
	{
		config = findByName(explorers, "ConeSprinter");
		skills = config["skills"].get<jarr_t>();

		auto sprintConfig = findByName(skills, "Sprint");
		e->mSkills[SPRINT_SKILL_INDEX] = createExplorerSkill(DoSprint, KEYCODE_Q, sprintConfig);
		e->mSkills[SPRINT_SKILL_INDEX]->mSceneObject = e;

		auto meleeConfig = findByName(skills, "ConeAttack");
		e->mSkills[MELEE_SKILL_INDEX] = createExplorerSkill(DoMelee, MOUSEBUTTON_LEFT, meleeConfig);
		e->mSkills[MELEE_SKILL_INDEX]->mSceneObject = e;

		UIManager* mUIManager = &Application::SharedInstance().GetCurrentScene()->mUIManager;
		mUIManager->AddSkill(e->mSkills[MELEE_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 5, 8);
		mUIManager->AddSkill(e->mSkills[SPRINT_SKILL_INDEX], SPRITESHEET_EXPLORER_ICONS, 1, 6);

		e->mAttackDamage = meleeConfig["damage"].get<float>();
		e->mController->mSprintMultiplier = sprintConfig["speedMultiplier"].get<float>();

		break;
	}
	}

	e->mController->mBaseMoveSpeed = config["moveSpeed"].get<float>() / 10;
	e->mHealth->SetMaxHealth(config["baseHealth"].get<float>());
}

void Explorer::OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->UpdateComponents(newRot, newPos);

	if (Singleton<NetworkManager>::SharedInstance().mMode == NetworkManager::SERVER)
	{
		auto& ai = Singleton<AIManager>::SharedInstance();
		Node* node = ai.GetNodeAt(newPos);

		if (node != e->mCurrentNode)
		{
			e->mCurrentNode = node;
			ai.SetGridDirty(true);
		}
	}
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

void Explorer::OnAnimStateChange(BaseSceneObject* obj, AnimationControllerState prevState, AnimationControllerState newState)
{
	if (newState != ANIM_STATE_MELEE)
	{
		auto e = reinterpret_cast<Explorer*>(obj);
		if (e->mNetworkID->mHasAuthority)
		{
			e->mMeleeColliderComponent.asBaseColliderComponent->mIsActive = false;
		}
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

void Explorer::OnHealthChange(BaseSceneObject* obj, float oldVal, float newVal, float hitDirection)
{

}

void Explorer::OnDeath(BaseSceneObject* obj)
{
	Explorer* pExplorer = reinterpret_cast<Explorer*>(obj);
	pExplorer->mIsDead = true;
	pExplorer->mInteractionCollider->mIsActive = true;
	pExplorer->mCollider->mIsDynamic = false;
	pExplorer->mController->PlayStateAnimation(ANIM_STATE_DEATH);
	Singleton<AIManager>::SharedInstance().SetGridDirty(true);
}

void Explorer::OnRevive(BaseSceneObject* obj)
{
	Explorer* pExplorer = reinterpret_cast<Explorer*>(obj);
	pExplorer->mIsDead = false;
	pExplorer->mInteractionCollider->mIsActive = false;
	pExplorer->mCollider->mIsDynamic = true;
	pExplorer->mController->PlayStateAnimation(ANIM_STATE_IDLE);
	Singleton<AIManager>::SharedInstance().SetGridDirty(true);
}

void Explorer::OnCollisionExit(BaseSceneObject* obj, BaseSceneObject* other)
{
	auto e = static_cast<Explorer*>(obj);
	e->OnMove(e, e->mTransform->GetPosition(), e->mTransform->GetRotation());
}

void Explorer::OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
{
	// We don't want to receive this msg for our base collider.
	if (obj == other) 
	{
		return;
	}

	if (other->Is<Explorer>())
	{
		Explorer* pOtherExplorer = reinterpret_cast<Explorer*>(other);
		if (pOtherExplorer->mController->mIsInteracting)
		{
			Explorer* pThisExplorer = reinterpret_cast<Explorer*>(obj);
			pThisExplorer->mHealth->SetHealth(pThisExplorer->mHealth->GetMaxHealth() * 0.3f, false);
			pOtherExplorer->mController->ConsumeInteractWill();
		}
	}
}

void Explorer::UpdateComponents(quatf rotation, vec3f position)
{
	mCollider->mCollider.origin = position + (rotation * mCollider->mOffset);
	mInteractionCollider->mCollider.origin = mCollider->mCollider.origin;
	
	// Add more cases as we add explorer types
	switch (GetExplorerType())
	{
	case TRAPMASTER:
	case HEALER:
	case SPRINTER:
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
		p.AsSkill.Position = explorer->mTransform->GetPosition() - vec3f(0.0f, 0.0f, 2.5f);
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

bool Explorer::DoLantern(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	Explorer* explorer = reinterpret_cast<Explorer*>(obj);
	if (explorer->mNetworkID->mHasAuthority)
	{
		Packet p(PacketTypes::SPAWN_SKILL);
		p.AsSkill.Position = explorer->mTransform->GetPosition();
		p.AsSkill.Duration = explorer->mSkills[LANTERN_SKILL_INDEX]->mDuration;
		p.AsSkill.Type = SkillPacketTypes::SKILL_TYPE_LANTERN;
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
	auto e = reinterpret_cast<Explorer*>(self);

	// THis is currently an assumption that this object will have a Health component
	if (other->Is<Minion>())
	{
		auto m = reinterpret_cast<Minion*>(other);
		vec3f dir = other->mTransform->GetPosition() - self->mTransform->GetPosition();
		m->mHealth->TakeDamage(e->mAttackDamage, atan2f(dir.y, dir.x), false);
	}
	else if (other->Is<Explorer>())
	{

		// Check Friendly Fire or haunt state of explorer.
		//	auto e = reinterpret_cast<Explorer*>(other);
		//	e->mHealth->TakeDamage(100.0f, false);
	}
}