#include <stdafx.h>
#include <Components/AnimationUtility.h>
#include <Components/AnimationController.h>

Animation gSprinterAnimations[Animations::SPRINTER_ANIMATION_COUNT] = {
	{ "Sprinter_Mix_Finish", 10, 46, 1 },
	{ "Sprinter_Mix_Finish", 50, 74, 1 },
	{ "Sprinter_Mix_Finish", 80, 102, 1 }
};

Animation gMinionAnimations[Animations::MINION_ANIMATION_COUNT] = {
	{ "Minion_01_Animation_Pass_1_1_1.0007", 10,  60,  1.8f },
	{ "Minion_01_Animation_Pass_1_1_1.0007", 80,  110, 1 },
	{ "Minion_01_Animation_Pass_1_1_1.0007", 130, 159, 1 }
};

Animation gPlantAnimations[Animations::PLANT_ANIMATION_COUNT] = {
	{ "Flytrap_Export_Version", 5,		120,	1 },
	{ "Flytrap_Export_Version", 120,	200,	1 },
	{ "Flytrap_Export_Version", 230,	310,	1 },
};

uint32_t gMinionRestFrameIndex = 5;

void SetRestFrameIndex(AnimationController* animationController, uint32_t index)
{
	animationController->SetRestFrameIndex(index);
}

void PlayAnimation(AnimationController* animationController, Animation* animation, bool shouldLoop)
{
	animationController->PlayAnimationRange(animation->takeName, animation->startFrameIndex, animation->endFrameIndex, nullptr, 0, shouldLoop);
}

void PlayAnimation(AnimationController* animationController, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop)
{
	animationController->PlayAnimationRange(animation->takeName, animation->startFrameIndex, animation->endFrameIndex, options, count, shouldLoop);
}

void SetStateAnimation(AnimationController* animationController, char state, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop)
{
	animationController->SetStateAnimation(static_cast<AnimationControllerState>(state), animation->takeName, animation->startFrameIndex, animation->endFrameIndex, animation->speed, options, count, shouldLoop);
}