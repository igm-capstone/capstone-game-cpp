#pragma once
#include "Components/BaseComponent.h"
#include "SkeletalHierarchy.h"

typedef void(*OnKeyframe)(void* caller);

struct KeyframeOption
{
	uint32_t index;
	OnKeyframe onKeyframe;
};

class AnimationController :
	public BaseComponent
{
private:
	friend class Factory<AnimationController>;

	int				mCurrentAnimationIndex;
	uint32_t		mCurrentAnimationStartIndex;
	uint32_t		mCurrentAnimationEndIndex;

	float			mCurrentAnimationPlayTime;

	AnimationController();
	~AnimationController();

	int FindAnimationIndex(const char* name);
	void SetKeyframeOptions(KeyframeOption* options, uint32_t count);
public:
	std::vector<SkeletalAnimation>* mSkeletalAnimations;
	std::unordered_map<int, OnKeyframe> mKeyframeCallbackMap;

	SkeletalHierarchy mSkeletalHierarchy;

	bool mIsAnimating;
	bool mIsLooping;

	void PlayAnimation(const char* name, bool shoudLoop = false);
	void PlayAnimation(const char* name, KeyframeOption* options, uint32_t count, bool shoudLoop = false);

	void PlayAnimationRange(const char* name, uint32_t startIndex, uint32_t endIndex, bool shoudLoop = false);

	void Update(double milliseconds);
};

