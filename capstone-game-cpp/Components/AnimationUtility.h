#pragma once
#include <stdint.h>


namespace Animations
{
	enum SprinterAnimations : int
	{
		SPRINTER_IDLE,
		SPRINTER_RUN,
		SPRINTER_ATTACK,
		SPRINTER_ANIMATION_COUNT
	};

	enum MinionAnimations: int
	{
		MINION_WALK,
		MINION_RUN,
		MINION_ATTACK,
		MINION_ANIMATION_COUNT
	};

	enum PlantAnimations : int
	{
		PLANT_IDLE,
		PLANT_BITE,
		PLANT_SHOOT,
		PLANT_ANIMATION_COUNT
	};

	enum TrapAnimiations : int
	{
		TRAP_IDLE,
		TRAP_OPEN,
		TRAP_ANIMATION_COUNT
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

extern Animation gSprinterAnimations[Animations::SPRINTER_ANIMATION_COUNT];
extern Animation gMinionAnimations[Animations::MINION_ANIMATION_COUNT];
extern Animation gPlantAnimations[Animations::PLANT_ANIMATION_COUNT];
extern Animation gTrapAnimations[Animations::TRAP_ANIMATION_COUNT];

extern uint32_t gMinionRestFrameIndex;

void SetRestFrameIndex(AnimationController* animationController, uint32_t index);
void PlayAnimation(AnimationController* animationController, Animation* animation, bool shouldLoop = false);
void PlayAnimation(AnimationController* animationController, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop = false);
void SetStateAnimation(AnimationController* animationController, char state, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop = false);