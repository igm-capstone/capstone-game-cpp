#include "stdafx.h"
#include "AnimationController.h"


AnimationController::AnimationController() : mCurrentAnimationIndex(-1), mCurrentAnimationPlayTime(0.0f), mIsAnimating(false), mIsLooping(false)
{
}


AnimationController::~AnimationController()
{
}

void AnimationController::PlayAnimation(const char* name)
{
	int index = -1;
	for (uint32_t i = 0; i < mSkeletalAnimations.size(); i++)
	{
		if (strcmp(name, mSkeletalAnimations[i].name.c_str()) == 0)
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

	SkeletalAnimation* currentAnimation = &mSkeletalAnimations[mCurrentAnimationIndex];

	if (mCurrentAnimationPlayTime <= currentAnimation->duration)
	{
		Skeleton& skeletalHierarchy = mSkeleton;
		
		float framesPerMS = static_cast<float>(currentAnimation->frameCount) / currentAnimation->duration;

		uint32_t keyframeIndex = static_cast<int>(floorf(mCurrentAnimationPlayTime));


		float u = (mCurrentAnimationPlayTime - keyframeIndex);

		for (JointAnimation jointAnimation : currentAnimation->jointAnimations)
		{
			Keyframe& current	= jointAnimation.keyframes[keyframeIndex];
			Keyframe& next		= jointAnimation.keyframes[min(keyframeIndex + 1, currentAnimation->jointAnimations.size() - 1)];

			quatf rotation		= cliqCity::graphicsMath::normalize(cliqCity::graphicsMath::slerp(current.rotation, next.rotation, u));
			vec3f scale			= cliqCity::graphicsMath::lerp(current.scale, next.scale, u);
			vec3f translation	= cliqCity::graphicsMath::lerp(current.translation, next.translation, u);
			
			skeletalHierarchy.mJoints[jointAnimation.jointIndex].animPoseMatrix = mat4f::scale(scale) * rotation.toMatrix4() * mat4f::translate(translation);
		}

		skeletalHierarchy.UpdateAnimationPose();

		mCurrentAnimationPlayTime += static_cast<float>(milliseconds);
	}
	else
	{
		if (mIsLooping)
		{
			mCurrentAnimationPlayTime -= currentAnimation->duration;
		}
		else
		{
			mIsAnimating = false;
			mCurrentAnimationIndex = -1;
			mCurrentAnimationPlayTime = 0.0f;
		}
	}
}

