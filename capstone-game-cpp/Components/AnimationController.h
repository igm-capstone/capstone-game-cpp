#pragma once
#include "Components/BaseComponent.h"
#include "SkeletalHierarchy.h"

typedef void(*OnKeyframe)(void* caller);

struct KeyframeOption
{
	uint32_t	index;
	OnKeyframe	onKeyframe;
};

struct StateAnimation
{
	int									index = -1;
	uint32_t							startFrameIndex = 0;
	uint32_t							endFrameIndex	= 0;
	float                               speed;
	bool								shouldLoop = false;
	std::unordered_map<int, OnKeyframe> keyframeCallbackMap;
};

enum AnimationControllerState : char
{
	ANIM_STATE_INITIAL = 0,
	ANIM_STATE_IDLE,
	ANIM_STATE_WALK,
	ANIM_STATE_RUN,
	ANIM_STATE_MELEE,
	ANIM_STATE_MELEE_ALT,
	ANIM_STATE_SKILL_0,
	ANIM_STATE_SKILL_1,
	ANIM_STATE_SKILL_2,
	ANIM_STATE_REVIVE,
	ANIM_STATE_DEATH,
	ANIM_STATE_NULL,
	ANIM_STATE_COUNT
};

enum AnimationControllerCommand : byte
{
	ANIM_STATE_COMMAND_PLAY,
	ANIM_STATE_COMMAND_PAUSE,
};

class AnimationController :
	public BaseComponent
{
	EXPOSE_CALLBACK_2(CommandExecuted, AnimationControllerState, AnimationControllerCommand)
	EXPOSE_CALLBACK_2(StateChanged, AnimationControllerState, AnimationControllerState)

	friend class Factory<AnimationController>;

	uint32_t		mPreviousKeyframeIndex;
	uint32_t		mCurrentKeyframeIndex;
	int				mCurrentAnimationIndex;
	uint32_t		mCurrentAnimationStartIndex;
	uint32_t		mCurrentAnimationEndIndex;
	float			mCurrentAnimationSpeed;
	uint32_t		mRestIndex;
	float			mCurrentAnimationPlayTime;

	AnimationControllerState  mState;

	bool mIsAnimating;
	bool mIsLooping;
	bool mIsPaused;

	AnimationController();
	~AnimationController();

	int		FindAnimationIndex(const char* name);
	void	SetKeyframeOptions(KeyframeOption* options, uint32_t count);
	void	PlayAnimationRange(int atIndex, uint32_t startIndex, uint32_t endIndex, KeyframeOption* options, uint32_t count, bool shoudLoop);

public:
	std::vector<SkeletalAnimation>*									mSkeletalAnimations;
	std::unordered_map<AnimationControllerState, StateAnimation>	mStateAnimationMap;
	std::unordered_map<int, OnKeyframe>								mKeyframeCallbackMap;
	SkeletalHierarchy												mSkeletalHierarchy;

	inline void	SetRestFrameIndex(uint32_t index) { mRestIndex = index; }
	
	void SetState(AnimationControllerState state, bool forceRestart = false);
	void SetStateAnimation(AnimationControllerState state, const char* name, uint32_t startIndex, uint32_t endIndex,float speed, KeyframeOption* options, uint32_t count, bool shouldLoop);

	inline void Resume() {
		if (!mIsPaused) {
			return;
		}

		mIsPaused = false;
		OnCommandExecuted(mState, ANIM_STATE_COMMAND_PLAY);
	};

	inline void Pause() {
		if (mIsPaused) {
			return;
		}

		mIsPaused = true; 
		OnCommandExecuted(mState, ANIM_STATE_COMMAND_PAUSE);
	};

	inline AnimationControllerState GetState() { return mState; };
	inline bool IsPaused() { return mIsPaused; };


	void PlayAnimation(const char* name, bool shoudLoop = false);
	void PlayAnimation(const char* name, KeyframeOption* options, uint32_t count, bool shoudLoop = false);
	void PlayAnimationRange(const char* name, uint32_t startIndex, uint32_t endIndex, KeyframeOption* options, uint32_t count, bool shoudLoop = false);


	void Update(double milliseconds);
	void ExecuteKeyframeCallbacks();

	void UpdateAnimation(SkeletalAnimation* pCurrentAnimation, float milliseconds, float framesPerMS);
	void SetRestPose();
};