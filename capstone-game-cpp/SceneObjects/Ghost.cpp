#include "stdafx.h"
#include "Ghost.h"
#include <Components/FmodEventCollection.h>
#include <Components/Skill.h>
#include <Components/NetworkID.h>
#include <Components/GhostController.h>
#include <CameraManager.h>
#include "Door.h"
#include "Region.h"

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
	spawnMinion->Setup("Basic Minion", 10, 0, DoSpawnBasicMinion);
	mSkills[1] = spawnMinion;

	spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->Setup("Bomber Minion", 20, 0, DoSpawnBomberMinion);
	mSkills[2] = spawnMinion;

	spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->Setup("Plant Minion", 10, 0, DoSpawnPlantMinion);
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
}

void Ghost::Spawn(BaseScene * scene)
{
	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = -1;

	auto level = scene->mLevel;
	mCameraManager->MoveCamera(level.center, level.center + vec3f(0.0f, 0.0f, -85.5f));

	mSkillBar = &scene->mSkillBar;
	mSkillBar->AddSkill(mSkills[1], 1, 0);
	mSkillBar->AddSkill(mSkills[2], 1, 1);
	mSkillBar->AddSkill(mSkills[3], 1, 5);
	mSkillBar->AddSkill(mSkills[4], 1, 4);
	SetActiveSkill(1);
}

void Ghost::DoSpawnBasicMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	DoSpawnMinion(obj, duration, target, pos, 0);
}
void Ghost::DoSpawnBomberMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	DoSpawnMinion(obj, duration, target, pos, 1);
}
void Ghost::DoSpawnPlantMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	DoSpawnMinion(obj, duration, target, pos, 2);
}

void Ghost::DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos, int minionType)
{
	auto ghost = reinterpret_cast<Ghost*>(obj);
	ghost->mEvents->Play("Spawn");

	TRACE_LOG("Spawning at" << pos);
	NetworkCmd::SpawnNewMinion(pos, minionType);
}

void Ghost::DoTransmogrify(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	auto victim = reinterpret_cast<Explorer*>(target);

	//TODO
}

void Ghost::DoMouseClick(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	auto ghost = reinterpret_cast<Ghost*>(obj);

	if (target) {
		if (target->Is<Door>()) {
			auto door = reinterpret_cast<Door*>(target);
			door->ToogleDoor();
		}
		else if (target->Is<Region>()) {
			if (ghost->mActiveSkill <= 3)
				ghost->mSkills[ghost->mActiveSkill]->UseSkill(target, pos);
		}
		else if (target->Is<Explorer>()) {
			if (ghost->mActiveSkill == 3)
				ghost->mSkills[ghost->mActiveSkill]->UseSkill(target, pos);
		}
	}
}

void Ghost::SetActiveSkill(int skillNum)
{
	mActiveSkill = skillNum;
	mSkillBar->SetActive(mSkills[mActiveSkill]);
}