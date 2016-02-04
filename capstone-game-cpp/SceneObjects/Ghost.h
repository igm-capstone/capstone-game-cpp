#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Rig3D/Graphics/Camera.h>
#include <ScareTacticsApplication.h>

class Ghost : public BaseSceneObject
{
	friend class Factory<Ghost>;
public:
	NetworkID*					mNetworkID;

private:
	Camera*						mCamera;

private:
	Ghost() : mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mCamera = &Application::SharedInstance().GetCurrentScene()->mCamera; //FIXME: CurrentScene is still the previous scene during constructors...
		mCamera->SetViewMatrix(mat4f::lookAtLH(vec3f(0, 0, 0), vec3f(10.0f, 0.0f, -100.0f), vec3f(0, 1, 0)));
	}
	~Ghost() {};

public:
	void Spawn(vec3f pos, int UUID)
	{
		mNetworkID->mIsActive = true;
		mNetworkID->mUUID = -1;
	};
};
