#include "stdafx.h"
#include "Ghost.h"
#include <Components/FmodEventCollection.h>
#include <Components/Skill.h>
#include <Components/NetworkID.h>
#include <Components/GhostController.h>
#include <CameraManager.h>
#include "Door.h"
#include "Region.h"
#include "Lamp.h"

//#include <CameraManager.h>

Ghost::Ghost() : mNetworkID(nullptr)
{
	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;

	mCameraManager = &Singleton<CameraManager>::SharedInstance();

	mController = Factory<GhostController>::Create();
	mController->mSceneObject = this;

	mEvents = Factory<FmodEventCollection>::Create();
	mEvents->mSceneObject = this;
	mEvents->Register("Spawn", "Basic Minion/Idle", FmodEventType::FIRE_AND_FORGET);

	memset(mSkills, 0, sizeof(mSkills));

	auto spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->Setup("Imp", 10, 0, DoSpawnImpMinion, 10);
	mSkills[1] = spawnMinion;

	spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->Setup("Abomination", 20, 0, DoSpawnAbominationMinion, 20);
	mSkills[2] = spawnMinion;

	spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->Setup("Flytrap", 10, 0, DoSpawnFlytrapMinion, 50);
	mSkills[3] = spawnMinion;

	spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->Setup("Transmogrify", 40, 20, DoTransmogrify);
	mSkills[4] = spawnMinion;
	
	auto clickInteraction = Factory<Skill>::Create();
	clickInteraction->mSceneObject = this;
	clickInteraction->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
	clickInteraction->Setup("Left Click", 0, 0, DoMouseClick);
	mSkills[0] = clickInteraction;

	mMana = mMaxMana;
}

void Ghost::Spawn(BaseScene* scene)
{
	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = -1;

	auto level = scene->mLevel;
	mCameraManager->MoveCamera(level.center, level.center + vec3f(0.0f, 0.0f, -85.5f));

	mSkillBar = &scene->mSkillBar;
	mSkillBar->AddSkill(mSkills[1], SPRITESHEET_GHOST_ICONS, 0, 0);
	mSkillBar->AddSkill(mSkills[2], SPRITESHEET_GHOST_ICONS, 1, 1);
	mSkillBar->AddSkill(mSkills[3], SPRITESHEET_GHOST_ICONS, 5, 2);
	mSkillBar->AddSkill(mSkills[4], SPRITESHEET_GHOST_ICONS, 4, 9);
	SetActiveSkill(1);
}

bool Ghost::DoSpawnImpMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	return DoSpawnMinion(obj, duration, target, pos, SKILL_TYPE_IMP_MINION);
}
bool Ghost::DoSpawnAbominationMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	return DoSpawnMinion(obj, duration, target, pos, SKILL_TYPE_ABOMINATION_MINION);
}
bool Ghost::DoSpawnFlytrapMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	return DoSpawnMinion(obj, duration, target, pos, SKILL_TYPE_FLYTRAP_MINION);
}

bool Ghost::DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos, SkillPacketTypes minionType)
{
	if (!target->Is<Region>()) return false;

	auto ghost = reinterpret_cast<Ghost*>(obj);
	ghost->mEvents->Play("Spawn");

	TRACE_LOG("Spawning at " << pos);
	NetworkCmd::SpawnNewSkill(minionType, pos, duration);

	return true;
}

bool Ghost::DoTransmogrify(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	if (!target->Is<Explorer>()) return false;

	auto victim = reinterpret_cast<Explorer*>(target);

	//TODO
	return true;
}

bool Ghost::DoMouseClick(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	auto ghost = reinterpret_cast<Ghost*>(obj);

	if (!target) return false;

	if (target->Is<Door>()) {
		auto door = reinterpret_cast<Door*>(target);
		door->ToogleDoor();
	}
	if (target->Is<Lamp>()) {
		auto lamp = reinterpret_cast<Lamp*>(target);
		lamp->ToggleLamp();
	}
	else {
		//Spawn X or transmogrify
		Skill* activeSkill = ghost->mSkills[ghost->mActiveSkill];

		if (ghost->mMana >= activeSkill->mCost) {
			if (activeSkill->UseSkill(target, pos)) {
				ghost->mMana -= activeSkill->mCost;
			}
		} else {
			TRACE_LOG("No mana for skill");
		}
	}

	return true;
}

void Ghost::SetActiveSkill(int skillNum)
{
	mActiveSkill = skillNum;
	mSkillBar->SetActive(mSkills[mActiveSkill]);
}

void Ghost::TickMana(float milliseconds)
{
	mMana += mManaRegenPerS * (milliseconds / 1000);
	if (mMana > mMaxMana) mMana = mMaxMana;
	if (mMana < 0) mMana = 0;
}
