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
	spawnMinion->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
	spawnMinion->Setup(0, 0, DoSpawnMinion);
	mSkills[0] = spawnMinion;

	auto doorInteract = Factory<Skill>::Create();
	doorInteract->mSceneObject = this;
	doorInteract->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
	doorInteract->Setup(0, 0, DoDoorInteract);
	mSkills[1] = doorInteract;
}

void Ghost::Spawn(BaseScene * scene)
{
	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = -1;

	auto level = scene->mLevel;
	mCameraManager->MoveCamera(level.center, level.center + vec3f(0.0f, 0.0f, -85.5f));
}

void Ghost::DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	auto ghost = reinterpret_cast<Ghost*>(obj);

	if (target && target->Is<Region>()) {
		ghost->mEvents->Play("Spawn");

		TRACE_LOG("Spawning at" << pos);
		NetworkCmd::SpawnNewMinion(pos);
	}
}

void Ghost::DoDoorInteract(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	if (target && target->Is<Door>()) {
		auto door = reinterpret_cast<Door*>(target);
		door->ToogleDoor();
	}
}