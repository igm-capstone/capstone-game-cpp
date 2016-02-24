#include <stdafx.h>
#include <Components/AnimationUtility.h>
#include <Components/AnimationController.h>

Animation gMinionWalk{ "Minion_01_Animation_Pass_1_1_1.0007", 10,	60 };
Animation gMinionMelee{ "Minion_01_Animation_Pass_1_1_1.0007", 80,	110 };
Animation gMinionRun{ "Minion_01_Animation_Pass_1_1_1.0007", 130,	160 };

void PlayAnimation(AnimationController* animationController, Animation* animation, bool shouldLoop)
{
	animationController->PlayAnimationRange(animation->takeName, animation->startFrameIndex, animation->endFrameIndex, shouldLoop);
}

void PlayAnimation(AnimationController* animationController, Animation* animation, KeyframeOption* options, uint32_t count, bool shouldLoop)
{
	animationController->PlayAnimation(animation->takeName, options, count, shouldLoop);
}