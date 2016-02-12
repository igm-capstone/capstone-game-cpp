#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Components/Skill.h>
#include <Rig3D/Graphics/Camera.h>
#include "ScareTacticsApplication.h"
#include <Components/Health.h>

#define MAX_EXPLORERS        4
#define MAX_EXPLORER_SKILLS  5

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	IMesh*						mMesh;
	NetworkID*					mNetworkID;
	ExplorerController*			mController;
	SphereColliderComponent*	mCollider;
	Skill*						mSkills[MAX_EXPLORER_SKILLS];
	Health*						mHealth;

private:
	NetworkClient*				mNetworkClient;
	CameraManager*				mCameraManager;

private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) {
		mTransform->SetScale(0.005f);				// Temp until we get properly sized models
		mTransform->SetRotation(PI * -0.5, 0.0f, 0.0f);
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<ExplorerController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;
		mController->mSpeed = 0.05f;
		mController->RegisterMoveCallback(&OnMove);

		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mIsDynamic = true;
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;
		mCollider->RegisterCollisionExitCallback(&OnCollisionExit);
	
		mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
		mNetworkID->RegisterNetSyncTransformCallback(&OnNetSyncTransform);

		mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;
		mCameraManager = &Singleton<CameraManager>::SharedInstance();

		mHealth = Factory<Health>::Create();
		mHealth->mSceneObject = this;
		mHealth->SetMaxHealth(1000.0f);

		memset(mSkills, 0, sizeof(Skill*) * MAX_EXPLORER_SKILLS);

		auto sprint = Factory<Skill>::Create();
		sprint->mSceneObject = this;
		sprint->SetBinding(SkillBinding().Set(KEYCODE_A).Set(MOUSEBUTTON_LEFT));
		sprint->Setup(2, 1, ExplorerController::DoSprint);
		mSkills[0] = sprint;
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

	static void OnMove(BaseSceneObject* obj, vec3f newPos)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mTransform->SetPosition(newPos);
		e->mCollider->mCollider.origin = newPos;
		
		if (e->mNetworkID->mHasAuthority) {
			e->mCameraManager->ChangeLookAtTo(newPos);
			Packet p(PacketTypes::SYNC_TRANSFORM);
			p.UUID = e->mNetworkID->mUUID;
			p.Position = newPos;
			e->mNetworkClient->SendData(&p);
		}

		e->mHealth->TakeDamage(1.0f);
	}

	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mController->mIsActive = newAuth;
		e->mCameraManager->MoveCamera(e->mTransform->GetPosition(), e->mTransform->GetPosition() + vec3f(0,-10,-20));
	}

	static void OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mTransform->SetPosition(newPos);
		e->mCollider->mCollider.origin = newPos;
	}

	static void OnCollisionExit(BaseSceneObject* obj, Collision*)
	{
		auto e = static_cast<Explorer*>(obj);
		if (e->mNetworkID->mHasAuthority) {
			e->mCameraManager->ChangeLookAtTo(e->mTransform->GetPosition());
		}
	}
};
