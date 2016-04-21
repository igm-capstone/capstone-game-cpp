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
#include "Explorer.h"

//#include <CameraManager.h>

using namespace nlohmann;
using jarr_t = json::array_t;

json findByName(jarr_t& array, string name)
{
	for (json& child : array)
	{
		if (child["name"] == name) {
			return child;
		}
	}

	return json();
}

Skill* createSkill(string skillName, Skill::UseCallback callback, json& skillConfig)
{
	auto cooldown = skillConfig["cooldown"].get<float>();
	auto cost = skillConfig["cost"].get<float>();
	
	auto skill = Factory<Skill>::Create();
	skill->Setup("Imp", cooldown, 0, callback, cost);

	return skill;
}

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

	json& config = Application::SharedInstance().GetConfigJson()["ghost"];
	jarr_t skills = config["skills"].get<jarr_t>();

	json impConfig = findByName(skills, "Basic Minion");
	mSkills[1] = createSkill("Imp", DoSpawnImpMinion, impConfig);
	mSkills[1]->mSceneObject = this;

	json abominationConfig = findByName(skills, "AOE Bomber");
	mSkills[2] = createSkill("Abomination", DoSpawnAbominationMinion, abominationConfig);
	mSkills[2]->mSceneObject = this;

	json flytrapConfig = findByName(skills, "Flytrap");
	mSkills[3] = createSkill("Flytrap", DoSpawnFlytrapMinion, flytrapConfig);
	mSkills[3]->mSceneObject = this;

	json transmogrifyConfig = findByName(skills, "Haunt Explorer To Minion");
	mSkills[4] = createSkill("Transmogrify", DoTransmogrify, transmogrifyConfig);
	mSkills[4]->mSceneObject = this;
	mSkills[4]->mDuration = transmogrifyConfig["duration"].get<float>();
	
	auto clickInteraction = Factory<Skill>::Create();
	clickInteraction->mSceneObject = this;
	clickInteraction->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
	clickInteraction->Setup("Left Click", 0, 0, DoMouseClick);
	mSkills[0] = clickInteraction;

	mMaxMana = config["baseEnergy"].get<float>();
	mManaRegenFrequency = 1 / config["regenEnergyTick"].get<float>();
	auto manaRegenArray = config["regenEnergy"].get<jarr_t>();
	for (size_t i = 0; i < 4; i++)
	{
		mManaRegen[i] = manaRegenArray[i].get<float>();
	}

	mManaRegenLevel = 0;
	mMana = mMaxMana;
}

Ghost::~Ghost()
{
	Factory<NetworkID>::Destroy(mNetworkID);
	Factory<GhostController>::Destroy(mController);
	Factory<FmodEventCollection>::Destroy(mEvents);

	for (int i = 0; i < MAX_GHOST_SKILLS; i++)
	{
		Factory<Skill>::Destroy(mSkills[i]);
	}
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
		door->Interact();
	}
	if (target->Is<Lamp>()) {
		auto lamp = reinterpret_cast<Lamp*>(target);
		lamp->Interact();
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
	mMana += mManaRegen[mManaRegenLevel] * mManaRegenFrequency * milliseconds * 0.001f;
	if (mMana > mMaxMana) mMana = mMaxMana;
	if (mMana < 0) mMana = 0;
}
