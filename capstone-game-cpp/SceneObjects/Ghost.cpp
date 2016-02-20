#include "stdafx.h"
#include "Ghost.h"
#include <Components/FmodEventCollection.h>
#include <Components/Skill.h>
#include <Components/NetworkID.h>
#include <CameraManager.h>

//#include <CameraManager.h>

Ghost::Ghost() : mNetworkID(nullptr)
{
	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;

	mCameraManager = &Singleton<CameraManager>::SharedInstance();

	mEvents = Factory<FmodEventCollection>::Create();
	mEvents->mSceneObject = this;
	mEvents->Register("Spawn", "Basic Minion/Idle", FmodEventType::FIRE_AND_FORGET);

	memset(mSkills, 0, sizeof(mSkills));

	auto spawnMinion = Factory<Skill>::Create();
	spawnMinion->mSceneObject = this;
	spawnMinion->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
	spawnMinion->Setup(0, 0, DoSpawnMinion);
	mSkills[0] = spawnMinion;
}

void Ghost::Spawn(vec3f pos, int UUID)
{
	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = -1;
}

void Ghost::DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	auto ghost = reinterpret_cast<Ghost*>(obj);
	ghost->mEvents->Play("Spawn");

	NetworkCmd::SpawnNewMinion(pos);
}
