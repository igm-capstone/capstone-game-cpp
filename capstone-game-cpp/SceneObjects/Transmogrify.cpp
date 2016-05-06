#include "stdafx.h"
#include "Transmogrify.h"
#include "SceneObjects/Explorer.h"
#include "ModelManager.h"
#include "ScareTacticsApplication.h"
#include "Components/AnimationUtility.h"
#include "Components/NetworkID.h"

Transmogrify::Transmogrify() : mAnimationController(Factory<AnimationController>::Create()), mTargetAnimationController(nullptr), mTargetModelCluster(nullptr)
{
	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;

	Animation melee = gMinionAnimations[Animations::MINION_ATTACK];
	KeyframeOption meleeOptions[] = {
		{ melee.startFrameIndex + 10, &Explorer::OnMeleeStart },
		{ melee.endFrameIndex,        &Explorer::OnMeleeStop },
	};

	SetStateAnimation(mAnimationController, ANIM_STATE_MELEE, &gMinionAnimations[Animations::MINION_ATTACK], meleeOptions, 2, false);
	SetStateAnimation(mAnimationController, ANIM_STATE_IDLE, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_WALK, &gMinionAnimations[Animations::MINION_WALK], nullptr, 0, true);
	SetStateAnimation(mAnimationController, ANIM_STATE_RUN, &gMinionAnimations[Animations::MINION_RUN], nullptr, 0, true);
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);
}

Transmogrify::~Transmogrify()
{
	Factory<AnimationController>::Destroy(mAnimationController);
}

void Transmogrify::Spawn(float duration, int targetUUID)
{
	mDuration = duration;

	for (Explorer& e : Factory<Explorer>())
	{
		// Only affects clients who are not the target
		if (e.mNetworkID->mUUID == targetUUID && !e.mNetworkID->mHasAuthority)
		{
			Attach(&e);
		}
	}
}

void Transmogrify::Attach(Explorer* pExplorer)
{
	mTargetAnimationController = pExplorer->mAnimationController;
	mTargetAnimationController->mIsActive = false;

	mTargetModelCluster = pExplorer->mModel;

	pExplorer->mModel = mModel;
	pExplorer->mAnimationController = mAnimationController;
	pExplorer->mAnimationController->mSceneObject = pExplorer;
	mAnimationController->mIsActive = true;
	mAnimationController->SetState(mTargetAnimationController->GetState());

	mTransform->SetScale(pExplorer->mTransform->GetScale());
	pExplorer->mTransform->SetScale(0.3f); // Minion model scale
	pExplorer->mIsTransmogrified = true;
}

void Transmogrify::Detach()
{
	if (!mTargetAnimationController)
	{
		return;
	}

	Explorer* pExplorer = reinterpret_cast<Explorer*>(mTargetAnimationController->mSceneObject);
	pExplorer->mAnimationController = mTargetAnimationController;
	pExplorer->mAnimationController->mIsActive = true;
	pExplorer->mAnimationController->SetState(mAnimationController->GetState());
	mTargetAnimationController = nullptr;
	
	pExplorer->mModel = mTargetModelCluster;
	pExplorer->mTransform->SetScale(mTransform->GetScale());
	pExplorer->mIsTransmogrified = false;

	mAnimationController->mSceneObject = nullptr;
	mAnimationController->mIsActive = false;
}

void Transmogrify::Update(float seconds)
{
	mDuration -= seconds;
	if (mDuration <= 0.0f)
	{
		Detach();
		Factory<Transmogrify>::Destroy(this);
	}
}