#include "stdafx.h"
#include "FlyTrap.h"
#include <Components/ColliderComponent.h>
#include <Components/MinionController.h>
#include <ScareTacticsApplication.h>
#include <Components/AnimationUtility.h>
#include <Network\NetworkClient.h>
#include <Components/NetworkID.h>
#include <Components/FlyTrapController.h>

FlyTrap::FlyTrap()
{
	mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mIsDynamic = true;

	mController = Factory<FlyTrapController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;

	Application::SharedInstance().GetModelManager()->GetModel(kPlantModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;
	mAnimationController->RegisterCommandExecutedCallback(&OnAnimationCommandExecuted);

	Animation melee = gPlantAnimations[Animations::PLANT_BITE];
	KeyframeOption meleeOptions[] = { { melee.startFrameIndex, OnMeleeStart },{ melee.endFrameIndex, OnMeleeStop } };
	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gPlantAnimations[Animations::PLANT_IDLE], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gPlantAnimations[Animations::PLANT_BITE], meleeOptions, 2, false);
}


FlyTrap::~FlyTrap()
{
}

void FlyTrap::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;

	mAnimationController->SetState(ANIM_STATE_IDLE);
	mAnimationController->Resume();
}

void FlyTrap::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto m = static_cast<FlyTrap*>(obj);
	m->mController->mIsActive = newAuth;
}

void FlyTrap::OnAnimationCommandExecuted(BaseSceneObject* obj, AnimationControllerState state, AnimationControllerCommand command)
{

}
 
void FlyTrap::OnNetSyncAnimation(BaseSceneObject* obj, byte state, byte command)
{

}

void FlyTrap::OnMeleeStart(void* obj)
{

}

void FlyTrap::OnMeleeStop(void* obj)
{

}

void FlyTrap::OnMeleeHit(BaseSceneObject* self, BaseSceneObject* other)
{
	 
}