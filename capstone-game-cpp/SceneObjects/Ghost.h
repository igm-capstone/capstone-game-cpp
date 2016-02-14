#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Rig3D/Graphics/Camera.h>
#include <ScareTacticsApplication.h>
#include <Components/GhostController.h>
#include <Components/FmodEvent.h>
#include <Components/Skill.h>
#include <Components/FmodEventCollection.h>

class Ghost : public BaseSceneObject
{
	friend class Factory<Ghost>;
public:
	NetworkID*					mNetworkID;
	GhostController*			mController;
	Skill*						mSkills[MAX_GHOST_SKILLS];
	FmodEventCollection*		mEvents;

private:
	CameraManager*				mCameraManager;

private:
	Ghost() : mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<GhostController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;

		mCameraManager = &Singleton<CameraManager>::SharedInstance();
		mCameraManager->MoveCamera(vec3f(0, 0, 0), vec3f(0.0f, 0.0f, -100.0f));

		mEvents = Factory<FmodEventCollection>::Create();
		mEvents->mSceneObject = this;
		mEvents->Register("Spawn", "Explosions/Single Explosion", FmodEventType::FIRE_AND_FORGET);

		memset(mSkills, 0, sizeof(Skill*) * MAX_GHOST_SKILLS);

		auto spawnMinion = Factory<Skill>::Create();
		spawnMinion->mSceneObject = this;
		spawnMinion->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		spawnMinion->Setup(0, 0, GhostController::DoSpawnMinion);
		mSkills[0] = spawnMinion;
	}

	~Ghost() {};

public:
	void Spawn(vec3f pos, int UUID)
	{
		mNetworkID->mIsActive = true;
		mNetworkID->mUUID = -1;
	};
};
