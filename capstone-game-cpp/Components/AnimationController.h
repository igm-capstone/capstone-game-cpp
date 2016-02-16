#pragma once
#include "Components/BaseComponent.h"
#include "Skeleton.h"

struct Keyframe
{
	quatf rotation;
	vec3f scale;
	vec3f translation;
	float time;
};

struct JointAnimation
{
	uint32_t jointIndex;
	std::vector<Keyframe> keyframes;
};

struct SkeletalAnimation
{
	uint32_t	frameCount;
	float		duration;
	std::string name;
	std::vector<JointAnimation> jointAnimations;
};

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
	std::vector<SkeletalAnimation> mSkeletalAnimations;
	Skeleton mSkeleton;
	bool mIsAnimating;
	bool mIsLooping;

	void PlayAnimation(const char* name);
	void PlayLoopingAnimation(const char* name);

	void Update(double milliseconds);

};

