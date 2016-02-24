#include "stdafx.h"
#include "AnimationController.h"
#include "trace.h"
#include "Rig3D\Common\Input.h"
#include "limits.h"

AnimationController::AnimationController() : 
	mCurrentAnimationIndex(-1), 
	mCurrentAnimationStartIndex(0), 
	mCurrentAnimationEndIndex(0), 
	mCurrentAnimationPlayTime(0.0f), 
	mSkeletalAnimations(nullptr), 
	mIsAnimating(false), 
	mIsLooping(false)
{
}

AnimationController::~AnimationController()
{
}

int AnimationController::FindAnimationIndex(const char* name)
{
	for (uint32_t i = 0; i < (*mSkeletalAnimations).size(); i++)
	{
		if (strcmp(name, (*mSkeletalAnimations)[i].name.c_str()) == 0)
		{
			return static_cast<int>(i);
		}
	}

	return -1;
}

void AnimationController::SetKeyframeOptions(KeyframeOption* options, uint32_t count)
{
	mKeyframeCallbackMap.clear();

	for (uint32_t i = 0; i < count; i++)
	{
		mKeyframeCallbackMap[options[i].index] = options[i].onKeyframe;
	}
}

void AnimationController::PlayAnimation(const char* name, bool shoudLoop)
{
	SetKeyframeOptions(nullptr, 0);

	int index = FindAnimationIndex(name);

	SkeletalAnimation* pSkeletalAnimation = &(*mSkeletalAnimations)[index];

	mCurrentAnimationIndex = index;
	mCurrentAnimationStartIndex = 0;
	mCurrentAnimationEndIndex = pSkeletalAnimation->frameCount - 1;
	mIsLooping = shoudLoop;
}

void AnimationController::PlayAnimation(const char* name, KeyframeOption* options, uint32_t count, bool shoudLoop)
{
	SetKeyframeOptions(options, count);

	int index = FindAnimationIndex(name);

	SkeletalAnimation* pSkeletalAnimation = &(*mSkeletalAnimations)[index];

	mCurrentAnimationIndex = index;
	mCurrentAnimationStartIndex = 0;
	mCurrentAnimationEndIndex = pSkeletalAnimation->frameCount - 1;
	mIsLooping = shoudLoop;
}

void AnimationController::PlayAnimationRange(const char* name, uint32_t startIndex, uint32_t endIndex, bool shoudLoop)
{
	SetKeyframeOptions(nullptr, 0);

	assert(startIndex < endIndex);

	int index = FindAnimationIndex(name);

	SkeletalAnimation* pSkeletalAnimation = &(*mSkeletalAnimations)[index];
	assert(startIndex < pSkeletalAnimation->frameCount && endIndex < pSkeletalAnimation->frameCount);
	
	mCurrentAnimationIndex		= index;
	mCurrentAnimationStartIndex = startIndex;
	mCurrentAnimationEndIndex	= endIndex;
	mIsLooping = shoudLoop;
}


void AnimationController::Update(double milliseconds)
{
	if (mCurrentAnimationIndex < 0)
	{
		return;
	}

	// So we can trigger frames at 0
	static uint32_t prevKeyframeIndex = UINT32_MAX;	

	SkeletalAnimation* currentAnimation = &(*mSkeletalAnimations)[mCurrentAnimationIndex];

	float framesPerMS	= static_cast<float>(currentAnimation->frameCount) / currentAnimation->duration;
	float duration		= (mCurrentAnimationEndIndex - mCurrentAnimationStartIndex + 1) / framesPerMS;

	if (mCurrentAnimationPlayTime <= duration)
	{
		SkeletalHierarchy& skeletalHierarchy = mSkeletalHierarchy;
		
		float t = mCurrentAnimationStartIndex + mCurrentAnimationPlayTime * framesPerMS;
		uint32_t keyframeIndex =static_cast<int>(floorf(t));
		//TRACE_LOG(prevKeyframeIndex << "\t" << keyframeIndex);

		float u = t - keyframeIndex;
		for (JointAnimation jointAnimation : currentAnimation->jointAnimations)
		{
			Keyframe& current	= jointAnimation.keyframes[keyframeIndex];
			Keyframe& next		= jointAnimation.keyframes[min(keyframeIndex + 1, mCurrentAnimationEndIndex)];

			quatf rotation		= cliqCity::graphicsMath::normalize(cliqCity::graphicsMath::slerp(current.rotation, next.rotation, u));
			vec3f scale			= cliqCity::graphicsMath::lerp(current.scale, next.scale, u);
			vec3f translation	= cliqCity::graphicsMath::lerp(current.translation, next.translation, u);
			
			skeletalHierarchy.mJoints[jointAnimation.jointIndex].animPoseMatrix =  mat4f::scale(scale) * rotation.toMatrix4() * mat4f::translate(translation);
		}

		mCurrentAnimationPlayTime += static_cast<float>(milliseconds);

		if (mKeyframeCallbackMap[keyframeIndex] != nullptr && prevKeyframeIndex != keyframeIndex)
		{ 
			mKeyframeCallbackMap[keyframeIndex](mSceneObject);
		}

		prevKeyframeIndex = keyframeIndex;
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

