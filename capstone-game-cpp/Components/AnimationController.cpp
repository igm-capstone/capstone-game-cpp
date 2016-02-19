#include "stdafx.h"
#include "AnimationController.h"
#include "trace.h"
#include "Rig3D\Common\Input.h"
#include <ModelManager.h>

AnimationController::AnimationController() : mCurrentAnimationIndex(-1), mCurrentAnimationPlayTime(0.0f), mIsAnimating(false), mIsLooping(false)
{
}


AnimationController::~AnimationController()
{
}

void AnimationController::PlayAnimation(const char* name)
{
	int index = -1;
	for (uint32_t i = 0; i < mSceneObject->GetModelCluster()->mSkeletalAnimations.size(); i++)
	{
		if (strcmp(name, mSceneObject->GetModelCluster()->mSkeletalAnimations[i].name.c_str()) == 0)
		{
			index = static_cast<int>(i);
		}
	}

	mCurrentAnimationIndex = index;
}

void AnimationController::PlayLoopingAnimation(const char* name)
{
	mIsLooping = true;
	PlayAnimation(name);
}

void AnimationController::Update(double milliseconds)
{
	if (mCurrentAnimationIndex < 0)
	{
		return;
	}
	static int f = 0;

	

	SkeletalAnimation* currentAnimation = &mSceneObject->GetModelCluster()->mSkeletalAnimations[mCurrentAnimationIndex];

	//Input* input = Singleton<Engine>::SharedInstance().GetInput();
	//if (input->GetKeyDown(KEYCODE_RIGHT))
	//{
	//	f += 10;
	//	if (f > currentAnimation->frameCount)
	//	{
	//		f = 0;
	//	}
	//}

	float duration = currentAnimation->duration;

	if (mCurrentAnimationPlayTime <= duration)
	{
		SkeletalHierarchy& skeletalHierarchy = mSceneObject->GetModelCluster()->mSkeletalHierarchy;
		
		float framesPerMS = static_cast<float>(currentAnimation->frameCount) / duration;

	//	uint32_t keyframeIndex = f;
		uint32_t keyframeIndex = static_cast<int>(floorf(mCurrentAnimationPlayTime * framesPerMS));

		float u = 0;// (mCurrentAnimationPlayTime - keyframeIndex);

		for (JointAnimation jointAnimation : currentAnimation->jointAnimations)
		{
			Keyframe& current	= jointAnimation.keyframes[keyframeIndex];
			Keyframe& next		= jointAnimation.keyframes[min(keyframeIndex + 1, currentAnimation->jointAnimations.size() - 1)];

			quatf rotation		= cliqCity::graphicsMath::normalize(cliqCity::graphicsMath::slerp(current.rotation, next.rotation, u));
			vec3f scale			= cliqCity::graphicsMath::lerp(current.scale, next.scale, u);
			vec3f translation	= cliqCity::graphicsMath::lerp(current.translation, next.translation, u);
			
			skeletalHierarchy.mJoints[jointAnimation.jointIndex].animPoseMatrix =  mat4f::scale(scale) * rotation.toMatrix4() * mat4f::translate(translation);
		}

		mCurrentAnimationPlayTime += static_cast<float>(milliseconds);
	}
	else
	{
		if (mIsLooping)
		{
			mCurrentAnimationPlayTime -= duration;
		}
		else
		{
			mIsAnimating = false;
			mCurrentAnimationIndex = -1;
			mCurrentAnimationPlayTime = 0.0f;
		}
	}
}

