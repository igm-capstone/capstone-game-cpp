#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components\NetworkID.h"
#include "Factory.h"
#include <Components/ColliderComponent.h>
#include <Components/ExplorerController.h>
#include <Components/Skill.h>
#include "ScareTacticsApplication.h"
#include <Components/Health.h>
#include <Components/AnimationController.h>

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	IMesh*						mMesh;
	NetworkID*					mNetworkID;
	ExplorerController*			mController;
	AnimationController*		mAnimationController;
	SphereColliderComponent*	mCollider;
	Skill*						mSkills[MAX_EXPLORER_SKILLS];
	Health*						mHealth;

private:
	NetworkClient*				mNetworkClient;
	CameraManager*				mCameraManager;

private:
	Explorer() : mMesh(nullptr), mNetworkID(nullptr) 
	{
		mNetworkID = Factory<NetworkID>::Create();
		mNetworkID->mSceneObject = this;
		mNetworkID->mIsActive = false;

		mController = Factory<ExplorerController>::Create();
		mController->mSceneObject = this;
		mController->mIsActive = false;
		mController->mSpeed = 0.05f;
		mController->RegisterMoveCallback(&OnMove);
		mController->SetBaseRotation(PI * 0.5, PI, 0.0f);

		mAnimationController = Factory<AnimationController>::Create();
		mAnimationController->mSceneObject = this;

		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mIsDynamic = true;
		mCollider->mSceneObject = this;
		mCollider->mIsActive = false;
		mCollider->RegisterCollisionExitCallback(&OnCollisionExit);
	
		mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
		mNetworkID->RegisterNetSyncTransformCallback(&OnNetSyncTransform);
		mNetworkID->RegisterNetHealthChangeCallback(&OnNetHealthChange);

		mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;
		mCameraManager = &Singleton<CameraManager>::SharedInstance();

		mHealth = Factory<Health>::Create();
		mHealth->mSceneObject = this;
		mHealth->SetMaxHealth(1000.0f);
		mHealth->RegisterHealthChangeCallback(OnHealthChange);

		memset(mSkills, 0, sizeof(Skill*) * MAX_EXPLORER_SKILLS);

		auto sprint = Factory<Skill>::Create();
		sprint->mSceneObject = this;
		sprint->SetBinding(SkillBinding().Set(KEYCODE_A).Set(MOUSEBUTTON_LEFT));
		sprint->Setup(2, 1, DoSprint);
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

	static void OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mTransform->SetPosition(newPos);
		e->mTransform->SetRotation(newRot);
		e->mCollider->mCollider.origin = newPos;
		
		if (e->mNetworkID->mHasAuthority) {
			e->mCameraManager->ChangeLookAtTo(newPos);
			Packet p(PacketTypes::SYNC_TRANSFORM);
			p.UUID = e->mNetworkID->mUUID;
			p.Position = newPos;
			e->mNetworkClient->SendData(&p);

			e->mHealth->TakeDamage(1.0f);
			if (e->mHealth->GetHealth() <= 0) e->mHealth->TakeDamage(-1000.0f);
		}
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

	static void OnNetHealthChange(BaseSceneObject* obj, float newVal)
	{
		auto e = static_cast<Explorer*>(obj);
		e->mHealth->SetHealth(newVal);
	}

	static void OnHealthChange(BaseSceneObject* obj, float newVal)
	{
		auto e = static_cast<Explorer*>(obj);
		if (e->mNetworkID->mHasAuthority) {
			Packet p(PacketTypes::SYNC_HEALTH);
			p.UUID = e->mNetworkID->mUUID;
			p.Value = newVal;
			e->mNetworkClient->SendData(&p);
		}
	}

	static void OnCollisionExit(BaseSceneObject* obj, Collision*)
	{
		auto e = static_cast<Explorer*>(obj);
		if (e->mNetworkID->mHasAuthority) {
			e->mCameraManager->ChangeLookAtTo(e->mTransform->GetPosition());
		}
	}

	static void DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
	{
		TRACE_LOG("Sprint!!");

		auto e = reinterpret_cast<Explorer*>(obj);
		e->mController->Sprint(duration);
	}
};
