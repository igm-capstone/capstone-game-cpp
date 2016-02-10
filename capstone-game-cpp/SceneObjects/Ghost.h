#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Rig3D/Graphics/Camera.h>
#include <ScareTacticsApplication.h>
#include <Components/GhostController.h>

#define MAX_GHOST_SKILLS 4

class Ghost : public BaseSceneObject
{
	friend class Factory<Ghost>;
public:
	NetworkID*					mNetworkID;
	GhostController*			mController;
	Skill*						mSkills[MAX_GHOST_SKILLS];

private:
	Camera*						mCamera;

private:
	Ghost() : mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<GhostController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;

		mCamera = &Application::SharedInstance().GetCurrentScene()->mCamera; //FIXME: CurrentScene is still the previous scene during constructors...
		mCamera->SetViewMatrix(mat4f::lookAtLH(vec3f(0, 0, 0), vec3f(10.0f, 0.0f, -100.0f), vec3f(0, 1, 0)));

		memset(mSkills, 0, sizeof(Skill*) * MAX_GHOST_SKILLS);

		auto spawnMinion = Factory<Skill>::Create();
		spawnMinion->mSceneObject = this;
		spawnMinion->SetBinding(SkillBinding().Set(MOUSEBUTTON_LEFT));
		spawnMinion->Setup(2, 0, GhostController::DoSpawnMinion);
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
