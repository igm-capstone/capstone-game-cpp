#pragma once
#include "Components/BaseComponent.h"
#include "SkeletalHierarchy.h"

class AnimationController :
	public BaseComponent
{
private:
	friend class Factory<AnimationController>;

	int		mCurrentAnimationIndex;
	float	mCurrentAnimationPlayTime;

	AnimationController();
	~AnimationController();
public:
	std::vector<SkeletalAnimation>* mSkeletalAnimations;
	SkeletalHierarchy mSkeletalHierarchy;

	bool mIsAnimating;
	bool mIsLooping;

	void PlayAnimation(const char* name);
	void PlayLoopingAnimation(const char* name);

	void Update(double milliseconds);

};

