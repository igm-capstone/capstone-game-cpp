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
#include "jsonUtils.h"

using namespace nlohmann;
using jarr_t = json::array_t;

Skill* createGhostSkill(Skill::UseCallback callback, json& skillConfig)
{
	auto cooldown = skillConfig["cooldown"].get<float>();
	auto cost = skillConfig["cost"].get<float>();
	auto name = skillConfig["description"].get<string>();
	auto duration = skillConfig.find("duration") == skillConfig.end() ? 0.0f : skillConfig["duration"].get<float>();

	auto skill = Factory<Skill>::Create();
	skill->Setup(name, cooldown, duration, callback, cost);

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

	json impConfig = findByName(skills, "SpawnMinion");
	mSkills[1] = createGhostSkill(DoSpawnImpMinion, impConfig);
	mSkills[1]->mSceneObject = this;

	json abominationConfig = findByName(skills, "SpawnAOE");
	mSkills[2] = createGhostSkill(DoSpawnAbominationMinion, abominationConfig);
	mSkills[2]->mSceneObject = this;

	json flytrapConfig = findByName(skills, "SpawnPlant");
	mSkills[3] = createGhostSkill(DoSpawnFlytrapMinion, flytrapConfig);
	mSkills[3]->mSceneObject = this;

	json transmogrifyConfig = findByName(skills, "Haunt_ExpToMinion");
	mSkills[4] = createGhostSkill(DoTransmogrify, transmogrifyConfig);
	mSkills[4]->mSceneObject = this;
	mSkills[4]->mDuration = transmogrifyConfig["duration"].get<float>();
	
	auto clickInteraction = Factory<Skill>::Create();
	clickInteraction->mSceneObject = this;
	clickInteraction->SetBinding(MOUSEBUTTON_LEFT);
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
	mNetworkID->mHasAuthority = true;
	mNetworkID->mUUID = -1;

	auto level = scene->mLevel;
	mCameraManager->MoveCamera(level.center, level.center + vec3f(0.0f, 0.0f, -85.5f));

	mUIManager = &scene->mUIManager;
	mUIManager->AddSkill(mSkills[1], SPRITESHEET_GHOST_ICONS, 0, 0, 0, [this]() { this->SetActiveSkill(1); return true; });
	mUIManager->AddSkill(mSkills[2], SPRITESHEET_GHOST_ICONS, 1, 1, 1, [this]() { this->SetActiveSkill(2); return true; });
	mUIManager->AddSkill(mSkills[3], SPRITESHEET_GHOST_ICONS, 5, 2, 2, [this]() { this->SetActiveSkill(3); return true; });
	mUIManager->AddSkill(mSkills[4], SPRITESHEET_GHOST_ICONS, 4, 9, 3, [this]() { this->SetActiveSkill(4); return true; });
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
	char optional[3];

	NetworkCmd::SpawnNewSkill(minionType, pos, duration);

	return true;
}

bool Ghost::DoTransmogrify(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	if (!target->Is<Explorer>()) return false;

	Ghost* pGhost = reinterpret_cast<Ghost*>(obj);
	auto victim = reinterpret_cast<Explorer*>(target);
	NetworkCmd::SpawnNewSkill(SKILL_TYPE_TRANSMOGRIFY, victim->mTransform->GetPosition(), pGhost->mSkills[4]->mDuration, victim->mNetworkID->mUUID);

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

int Ghost::GetActiveSkill()
{
	return mActiveSkill;
}

void Ghost::SetActiveSkill(int skillNum)
{
	mActiveSkill = skillNum;
	mUIManager->SetActiveSkill(mSkills[mActiveSkill]);
}

void Ghost::TickMana(float milliseconds)
{
	mMana += mManaRegen[mManaRegenLevel] * mManaRegenFrequency * milliseconds * 0.001f;
	if (mMana > mMaxMana) mMana = mMaxMana;
	if (mMana < 0) mMana = 0;
}
