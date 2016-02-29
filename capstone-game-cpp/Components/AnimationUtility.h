#pragma once
#include <stdint.h>


namespace Animations
{
	enum MinionAnimations: int
	{
		MINION_WALK,
		MINION_RUN,
		MINION_ATTACK,
		MINION_ANIMATION_COUNT
	};

	enum ExplorerAnimations : int
	{
		EXPLORER_WALK,
		EXPLORER_RUN,
		EXPLORER_ATTACK,
		EXPLORER_ANIMATION_COUNT
	};
}

struct Animation
{
	const char* takeName;
	uint32_t startFrameIndex;
	uint32_t endFrameIndex;
	float speed;
};

class AnimationController;
struct KeyframeOption;

extern Animation gMinionAnimations[Animations::MINION_ANIMATION_COUNT];

extern uint32_t gMinionRestFrameIndex;

void SetRestFrameIndex(AnimationController* animationController, uint32_t index);
void PlayAnimation(AnimationController* animationController, Animation* animation, bool shouldLoop = false);
void PlayAnimation(AnimationController* animationController, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop = false);
void SetStateAnimation(AnimationController* animationController, char state, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop = false);