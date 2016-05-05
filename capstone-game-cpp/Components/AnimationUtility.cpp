#include <stdafx.h>
#include <Components/AnimationUtility.h>
#include <Components/AnimationController.h>
#include <SceneObjects/Explorer.h>
#include <ModelManager.h>
#include <ScareTacticsApplication.h>

Animation gSprinterAnimations[Animations::SPRINTER_ANIMATION_COUNT] = {
	{ "SprinterFinal.0004", 1, 70, 1 },
	{ "SprinterFinal.0004", 80, 92, 1 },
	{ "SprinterFinal.0004", 100, 150, 1 },
	{ "SprinterFinal.0004", 160, 217, 1 }
};

Animation gTrapperAnimations[Animations::TRAPPER_ANIMATION_COUNT] = {
	{ "SwootPants_Final.0002", 1, 55, 1 },
	{ "SwootPants_Final.0002", 60, 80, 1 },
	{ "SwootPants_Final.0002", 90, 125, 1 },
	{ "SwootPants_Final.0002", 140, 208, 1 }
};

Animation gProfessorAnimations[Animations::PROFESSOR_ANIMATION_COUNT] = {
	{ "Take 001", 1, 73, 1 },
	{ "Take 001", 80, 93, 1 },
	{ "Take 001", 105, 153, 1 },
	{ "Take 001", 165, 223, 1 },
	{ "Take 001", 245, 330, 1 }
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

Animation gTrapAnimations[Animations::TRAP_ANIMATION_COUNT] = {
	{ "Take 001",	1,	1, 1 },
	{ "Take 001",	2,	49, 2 },
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

void SetSprinterAnimations(Explorer* explorer)
{
	Application::SharedInstance().GetModelManager()->GetModel(kSprinterModelName)->Link(explorer);

	explorer->mAnimationController->mSkeletalAnimations = &explorer->mModel->mSkeletalAnimations;
	explorer->mAnimationController->mSkeletalHierarchy = explorer->mModel->mSkeletalHierarchy;
	explorer->mAnimationController->RegisterCommandExecutedCallback(&Explorer::OnAnimationCommandExecuted);

	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_IDLE, &gSprinterAnimations[Animations::SPRINTER_IDLE], nullptr, 0, true);
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_RUN, &gSprinterAnimations[Animations::SPRINTER_RUN], nullptr, 0, true);
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_DEATH, &gSprinterAnimations[Animations::SPRINTER_DEATH], nullptr, 0, false);

	Animation& melee = gSprinterAnimations[Animations::SPRINTER_ATTACK];
	KeyframeOption meleeOptions[] = { { melee.startFrameIndex, Explorer::OnMeleeStart },{ melee.endFrameIndex, Explorer::OnMeleeStop } };
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_MELEE, &gSprinterAnimations[Animations::SPRINTER_ATTACK], meleeOptions, 2, false);
}

void SetProfessorAnimations(class Explorer* explorer)
{
	Application::SharedInstance().GetModelManager()->GetModel(kProfessorModelName)->Link(explorer);

	explorer->mAnimationController->mSkeletalAnimations = &explorer->mModel->mSkeletalAnimations;
	explorer->mAnimationController->mSkeletalHierarchy = explorer->mModel->mSkeletalHierarchy;
	explorer->mAnimationController->RegisterCommandExecutedCallback(&Explorer::OnAnimationCommandExecuted);

	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_IDLE, &gProfessorAnimations[Animations::PROFESSOR_IDLE], nullptr, 0, true);
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_RUN, &gProfessorAnimations[Animations::PROFESSOR_RUN], nullptr, 0, true);

	Animation melee = gProfessorAnimations[Animations::PROFESSOR_ATTACK];
	KeyframeOption meleeOptions[] = { { melee.startFrameIndex, Explorer::OnMeleeStart },{ melee.endFrameIndex, Explorer::OnMeleeStop } };
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_MELEE, &gProfessorAnimations[Animations::PROFESSOR_ATTACK], meleeOptions, 2, false);
}

void SetTrapperAnimations(class Explorer* explorer)
{
	Application::SharedInstance().GetModelManager()->GetModel(kTrapperModelName)->Link(explorer);

	explorer->mAnimationController->mSkeletalAnimations = &explorer->mModel->mSkeletalAnimations;
	explorer->mAnimationController->mSkeletalHierarchy = explorer->mModel->mSkeletalHierarchy;
	explorer->mAnimationController->RegisterCommandExecutedCallback(&Explorer::OnAnimationCommandExecuted);

	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_IDLE, &gTrapperAnimations[Animations::TRAPPER_IDLE], nullptr, 0, true);
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_RUN, &gTrapperAnimations[Animations::TRAPPER_RUN], nullptr, 0, true);

	Animation melee = gTrapperAnimations[Animations::TRAPPER_THROW];
	KeyframeOption meleeOptions[] = { { melee.startFrameIndex, Explorer::OnMeleeStart },{ melee.endFrameIndex, Explorer::OnMeleeStop } };
	SetStateAnimation(explorer->mAnimationController, ANIM_STATE_MELEE, &gTrapperAnimations[Animations::TRAPPER_THROW], meleeOptions, 2, false);
}