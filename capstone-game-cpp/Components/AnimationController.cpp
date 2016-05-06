#include "stdafx.h"
#include "AnimationController.h"
#include "trace.h"
#include "Rig3D\Common\Input.h"
#include "limits.h"

AnimationController::AnimationController() : 
	mPreviousKeyframeIndex(0), 
	mCurrentKeyframeIndex(0),
	mCurrentAnimationIndex(-1), 
	mCurrentAnimationStartIndex(0), 
	mCurrentAnimationEndIndex(0), 
	mRestIndex(0),
	mCurrentAnimationPlayTime(0.0f), 
	mState(ANIM_STATE_INITIAL),
	mIsAnimating(false),
	mIsLooping(false),
	mIsPaused(false),
	mSkeletalAnimations(nullptr)
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

void AnimationController::SetStateAnimation(AnimationControllerState state, const char* name, uint32_t startIndex, uint32_t endIndex, float speed, KeyframeOption* options, uint32_t count, bool shouldLoop)
{
	if (mStateAnimationMap.find(state) == mStateAnimationMap.end())
	{
		mStateAnimationMap.insert({ state, StateAnimation() });
		
		StateAnimation* pStateAnimation = &mStateAnimationMap[state];
		pStateAnimation->index = FindAnimationIndex(name);
		pStateAnimation->startFrameIndex = startIndex;
		pStateAnimation->endFrameIndex = endIndex;
		pStateAnimation->speed = speed;
		pStateAnimation->shouldLoop = shouldLoop;
		
		for (uint32_t i = 0; i < count; i++)
		{
			pStateAnimation->keyframeCallbackMap.insert({ options[i].index, options[i].onKeyframe });
		}
	}
}

void AnimationController::SetState(AnimationControllerState state, bool forceRestart)
{
	if (mState == state && !forceRestart)
	{
		return;
	}

	auto prevState = mState;
	mState = state;

	if (mStateAnimationMap.find(mState) != mStateAnimationMap.end())
	{

		StateAnimation* pStateAnimation = &mStateAnimationMap[mState];

		mCurrentAnimationIndex		= pStateAnimation->index;
		mCurrentAnimationStartIndex = pStateAnimation->startFrameIndex;
		mCurrentAnimationEndIndex	= pStateAnimation->endFrameIndex;
		mCurrentAnimationSpeed      = pStateAnimation->speed;
		mIsLooping					= pStateAnimation->shouldLoop;
		mCurrentAnimationPlayTime	= 0.0f;
		mCurrentKeyframeIndex       = mCurrentAnimationStartIndex;
		mPreviousKeyframeIndex      = INT32_MAX;
	}
	else
	{
		mState = ANIM_STATE_NULL;
	}

	OnStateChanged(prevState, state);
	mIsPaused ? OnCommandExecuted(mState, ANIM_STATE_COMMAND_PAUSE) : OnCommandExecuted(mState, ANIM_STATE_COMMAND_PLAY);
}

void AnimationController::PlayAnimation(const char* name, bool shoudLoop)
{
	int index = FindAnimationIndex(name);

	PlayAnimationRange(index, 0, (*mSkeletalAnimations)[index].frameCount - 1, nullptr, 0, shoudLoop);
}

void AnimationController::PlayAnimation(const char* name, KeyframeOption* options, uint32_t count, bool shoudLoop)
{
	int index = FindAnimationIndex(name);

	PlayAnimationRange(index, 0, (*mSkeletalAnimations)[index].frameCount - 1, options, count, shoudLoop);
}

void AnimationController::PlayAnimationRange(const char* name, uint32_t startIndex, uint32_t endIndex, KeyframeOption* options, uint32_t count, bool shoudLoop)
{
	assert(startIndex < endIndex);

	int index = FindAnimationIndex(name);

	PlayAnimationRange(index, startIndex, endIndex, options, count, shoudLoop);
}

void AnimationController::PlayAnimationRange(int atIndex, uint32_t startIndex, uint32_t endIndex, KeyframeOption* options, uint32_t count, bool shoudLoop)
{
	SetKeyframeOptions(options, count);

	SkeletalAnimation* pSkeletalAnimation = &(*mSkeletalAnimations)[atIndex];
	assert(startIndex < pSkeletalAnimation->frameCount && endIndex < pSkeletalAnimation->frameCount);

	mCurrentAnimationIndex = atIndex;
	mCurrentAnimationStartIndex = startIndex;
	mCurrentAnimationEndIndex = endIndex;
	mIsLooping = shoudLoop;
}


void AnimationController::Update(double milliseconds)
{
	if (mCurrentAnimationIndex < 0)
	{
		return;
	}

	if (mIsPaused)
	{
		return;
	}

	if (!mIsAnimating) {
		mIsAnimating = true;
		OnCommandExecuted(mState, ANIM_STATE_COMMAND_PLAY);
	}

	// So we can trigger frames at 0

	SkeletalAnimation* currentAnimation = &(*mSkeletalAnimations)[mCurrentAnimationIndex];

	float framesPerMS	= (static_cast<float>(currentAnimation->frameCount) / currentAnimation->duration) * mCurrentAnimationSpeed;
	float duration		= (mCurrentAnimationEndIndex - mCurrentAnimationStartIndex + 1) / framesPerMS;

	if (mCurrentAnimationPlayTime <= duration)
	{
		UpdateAnimation(currentAnimation, static_cast<float>(milliseconds) * mCurrentAnimationSpeed, framesPerMS);
	}
	else
	{
		// execute keyframe callbacks from the previous keyframe index to the end
		// of the animation
		mPreviousKeyframeIndex = mCurrentKeyframeIndex;
		mCurrentKeyframeIndex = mCurrentAnimationEndIndex;
		
		ExecuteKeyframeCallbacks();

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

void AnimationController::ExecuteKeyframeCallbacks()
{
	StateAnimation* pStateAnimation = &mStateAnimationMap[mState];
	
	auto& callbackMap = pStateAnimation->keyframeCallbackMap;
	if (mPreviousKeyframeIndex < mCurrentKeyframeIndex)
	{
		for (size_t i = mPreviousKeyframeIndex + 1; i <= mCurrentKeyframeIndex; i++)
		{
			if (callbackMap.find(i) != callbackMap.end())
			{
				auto callback = callbackMap[i];
				callback(mSceneObject);
			}
		}
	}
	else if (mPreviousKeyframeIndex > mCurrentKeyframeIndex)
	{
		if (callbackMap.find(mCurrentKeyframeIndex) != callbackMap.end())
		{
			auto callback = callbackMap[mCurrentKeyframeIndex];
			callback(mSceneObject);
		}
	}
}

void AnimationController::UpdateAnimation(SkeletalAnimation* pCurrentAnimation, float milliseconds, float framesPerMS)
{
	//static uint32_t mPreviousKeyframeIndex = UINT32_MAX;

	SkeletalHierarchy& skeletalHierarchy = mSkeletalHierarchy;

	if (mCurrentAnimationPlayTime < 0) mCurrentAnimationPlayTime = 0;
	float t = mCurrentAnimationStartIndex + mCurrentAnimationPlayTime * framesPerMS;
	mCurrentKeyframeIndex = static_cast<int>(floorf(t));
	//TRACE_WATCH("Keyframe", mCurrentKeyframeIndex);

	float u = t - mCurrentKeyframeIndex;
	for (JointAnimation jointAnimation : pCurrentAnimation->jointAnimations)
	{
		Keyframe& current = jointAnimation.keyframes[min(mCurrentKeyframeIndex, mCurrentAnimationEndIndex)];
		Keyframe& next = jointAnimation.keyframes[min(mCurrentKeyframeIndex + 1, mCurrentAnimationEndIndex)];

		quatf rotation = cliqCity::graphicsMath::normalize(cliqCity::graphicsMath::slerp(current.rotation, next.rotation, u));
		vec3f scale = cliqCity::graphicsMath::lerp(current.scale, next.scale, u);
		vec3f translation = cliqCity::graphicsMath::lerp(current.translation, next.translation, u);

		skeletalHierarchy.mJoints[jointAnimation.jointIndex].animPoseMatrix = mat4f::scale(scale) * rotation.toMatrix4() * mat4f::translate(translation);
	}

	mCurrentAnimationPlayTime += static_cast<float>(milliseconds);

	ExecuteKeyframeCallbacks();

	mPreviousKeyframeIndex = mCurrentKeyframeIndex;
}

void AnimationController::SetRestPose()
{
	//SkeletalHierarchy& skeletalHierarchy = mSkeletalHierarchy;
	//for (JointAnimation jointAnimation : currentAnimation->jointAnimations)
	//{
	//	Keyframe& restFrame = jointAnimation.keyframes[mRestIndex];

	//	skeletalHierarchy.mJoints[jointAnimation.jointIndex].animPoseMatrix = mat4f::scale(restFrame.scale) * restFrame.rotation.toMatrix4() * mat4f::translate(restFrame.translation);
	//}
}


