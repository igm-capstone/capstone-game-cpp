#pragma once
#include "Components/BaseComponent.h"

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
	bool mIsAnimating;
	bool mIsLooping;

	void PlayAnimation(const char* name);
	void PlayLoopingAnimation(const char* name);

	void Update(double milliseconds);

};

