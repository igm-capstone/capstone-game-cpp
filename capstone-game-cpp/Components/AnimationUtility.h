#pragma once
#include <stdint.h>

struct Animation
{
	const char* takeName;
	uint32_t startFrameIndex;
	uint32_t endFrameIndex;
};

class AnimationController;
struct KeyframeOption;

extern Animation gMinionWalk;
extern Animation gMinionMelee;
extern Animation gMinionRun;

extern uint32_t gMinionRestFrameIndex;

void SetRestFrameIndex(AnimationController* animationController, uint32_t index);
void PlayAnimation(AnimationController* animationController, Animation* animation, bool shouldLoop = false);
void PlayAnimation(AnimationController* animationController, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop = false);
void SetStateAnimation(AnimationController* animationController, char state, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop = false);