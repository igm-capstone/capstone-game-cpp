#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Rig3D/Graphics/Camera.h>
#include "ScareTacticsApplication.h"

#define MAX_EXPLORERS 4

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	IMesh*						mMesh;
	NetworkID*					mNetworkID;
	ExplorerController*			mController;
	SphereColliderComponent*	mCollider;

private:
	NetworkClient*				mNetworkClient;
	Camera*						mCamera;

private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<ExplorerController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;
		mController->RegisterCallback(&OnControllerMove);

		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mTraits.isDynamic = true;
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;

		mNetworkID->RegisterCallback(&OnNetAuthorityChange);
		mNetworkID->RegisterCallback(&OnNetSyncTransform);

		mNetworkClient = &(Singleton<NetworkManager>::SharedInstance().mClient);
		mCamera = &Application::SharedInstance().GetCurrentScene()->mCamera;
	}
	~Explorer() {};

public:
	void Spawn(vec3f pos, int UUID)
	{
		mTransform->SetPosition(pos);		

		mCollider->mIsActive = true;
		mCollider->mCollider.origin = pos;

		mNetworkID->mIsActive = true;
		mNetworkID->mUUID = UUID;
	};

	static void OnControllerMove(BaseSceneObject* obj, vec3f newPos)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mTransform->SetPosition(newPos);
		e->mCollider->mCollider.origin = newPos;
		e->mCamera->SetViewMatrix(mat4f::lookAtLH(newPos, newPos - vec3f(0.0f, 0.0f, 20.0f), vec3f(0.0f, 1.0f, 0.0f)));
		
		if (e->mNetworkID->mHasAuthority) {
			Packet p(PacketTypes::SYNC_TRANSFORM);
			p.UUID = e->mNetworkID->mUUID;
			p.Position = newPos;
			e->mNetworkClient->SendData(&p);
		}

	}

	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mController->mIsActive = newAuth;
		e->mCamera->SetViewMatrix(mat4f::lookAtLH(e->mTransform->GetPosition(), e->mTransform->GetPosition() - vec3f(0.0f, 0.0f, 20.0f), vec3f(0.0f, 1.0f, 0.0f)));
	}

	static void OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mTransform->SetPosition(newPos);
		e->mCollider->mCollider.origin = newPos;
	}
};
