#include "stdafx.h"
#include "Minion.h"
#include <Components/NetworkID.h>
#include <Components/ColliderComponent.h>
#include <Components/MinionController.h>
#include <Components/AnimationController.h>
#include <ScareTacticsApplication.h>
#include <Vertex.h>


Minion::Minion()
{
	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mIsDynamic = true;

	mController = Factory<MinionController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->RegisterMoveCallback(&OnMove);
	
	Application::SharedInstance().GetModelManager()->GetModel("Models/Minion_Test")->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;
}

Minion::~Minion()
{
}

void Minion::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	mAnimationController->PlayLoopingAnimation("Minion_01_Animation_Pass_1_1_1.0007");
}

void Minion::OnMove(BaseSceneObject* obj, vec3f newPos)
{
	auto m = static_cast<Minion*>(obj);
	m->mTransform->SetPosition(newPos);
	m->mCollider->mCollider.origin = newPos;
}

void Minion::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto m = static_cast<Minion*>(obj);
	m->mController->mIsActive = newAuth;
}
