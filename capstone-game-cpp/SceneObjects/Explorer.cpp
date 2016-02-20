#include "stdafx.h"
#include "Explorer.h"
#include <Components/NetworkID.h>
#include <Components/ExplorerController.h>
#include <Components/AnimationController.h>
#include <Components/ColliderComponent.h>
#include <Components/Health.h>
#include <Components/Skill.h>
#include <Vertex.h>

Explorer::Explorer(): mMesh(nullptr)
{
	mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;
	mCameraManager = &Singleton<CameraManager>::SharedInstance();

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);
	mNetworkID->RegisterNetSyncTransformCallback(&OnNetSyncTransform);
	mNetworkID->RegisterNetHealthChangeCallback(&OnNetHealthChange);
	
	mController = Factory<ExplorerController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->mSpeed = 0.05f;
	mController->RegisterMoveCallback(&OnMove);
	mController->SetBaseRotation(PI * 0.5, PI, 0.0f);

	Application::SharedInstance().GetModelManager()->LoadModel<GPU::SkinnedVertex>("AnimTest");
	Application::SharedInstance().GetModelManager()->GetModel("AnimTest")->Link(this);
		
	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mIsDynamic = true;
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->RegisterCollisionExitCallback(&OnCollisionExit);

	mHealth = Factory<Health>::Create();
	mHealth->mSceneObject = this;
	mHealth->SetMaxHealth(1000.0f);
	mHealth->RegisterHealthChangeCallback(OnHealthChange);

	memset(mSkills, 0, sizeof(mSkills));

	auto sprint = Factory<Skill>::Create();
	sprint->mSceneObject = this;
	sprint->SetBinding(SkillBinding().Set(KEYCODE_A).Set(MOUSEBUTTON_LEFT));
	sprint->Setup(2, 1, DoSprint);
	mSkills[0] = sprint;
}

void Explorer::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);		

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	mAnimationController->PlayLoopingAnimation("Take 001");
}

void Explorer::OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->mCollider->mCollider.origin = newPos;
		
	if (e->mNetworkID->mHasAuthority) {
		e->mCameraManager->ChangeLookAtTo(newPos);
		Packet p(PacketTypes::SYNC_TRANSFORM);
		p.UUID = e->mNetworkID->mUUID;
		p.AsTransform.Position = newPos;
		p.AsTransform.Rotation = newRot;
		e->mNetworkClient->SendData(&p);

		e->mHealth->TakeDamage(1.0f);
		if (e->mHealth->GetHealth() <= 0) e->mHealth->TakeDamage(-1000.0f);
	}
}

void Explorer::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto e = static_cast<Explorer*>(obj);
	e->mController->mIsActive = newAuth;
	e->mCameraManager->MoveCamera(e->mTransform->GetPosition(), e->mTransform->GetPosition() + vec3f(0,-10,-20));
}

void Explorer::OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot)
{
	auto e = static_cast<Explorer*>(obj);
	e->mTransform->SetPosition(newPos);
	e->mTransform->SetRotation(newRot);
	e->mCollider->mCollider.origin = newPos;
}

void Explorer::OnNetHealthChange(BaseSceneObject* obj, float newVal)
{
	auto e = static_cast<Explorer*>(obj);
	e->mHealth->SetHealth(newVal);
}

void Explorer::OnHealthChange(BaseSceneObject* obj, float newVal)
{
	auto e = static_cast<Explorer*>(obj);
	if (e->mNetworkID->mHasAuthority) {
		Packet p(PacketTypes::SYNC_HEALTH);
		p.UUID = e->mNetworkID->mUUID;
		p.AsFloat = newVal;
		e->mNetworkClient->SendData(&p);
	}
}

void Explorer::OnCollisionExit(BaseSceneObject* obj, BaseSceneObject* other)
{
	auto e = static_cast<Explorer*>(obj);
	if (e->mNetworkID->mHasAuthority) {
		e->mCameraManager->ChangeLookAtTo(e->mTransform->GetPosition());
	}
}

void Explorer::DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	TRACE_LOG("Sprint!!");

	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->Sprint(duration);
}
