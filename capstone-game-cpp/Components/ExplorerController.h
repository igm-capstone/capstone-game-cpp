#pragma once
#include "BaseComponent.h"
#include <ScareTacticsApplication.h>
#include <Components/AnimationController.h>

class ExplorerController : public BaseComponent
{
	friend class Factory<ExplorerController>;

	ExplorerController();
	~ExplorerController();

	class Input* mInput;
	Application* mApplication;
	CameraManager* mCameraManager;
	
	double mSprintDuration;

	float mAcceleration;
	float mBaseMoveSpeed;
	float mSpeedMultiplier;
	vec3f mCurrentSpeed;
	void UpdateInteractWill();
	
	// delta time in seconds
	bool Move(float dt, vec3f& pos);
	bool Rotate(float dt, vec3f& pos, quatf& rot);
	bool CanMove();

public:
	class AnimationController* mAnimationController;
	float mSpeed;
	bool  mIsInteracting;

	bool Update(double milliseconds);
	void Sprint(float duration);
	void Melee();

	void PlayStateAnimation(AnimationControllerState state);
	void PauseStateAnimation(AnimationControllerState state);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
	EXPOSE_CALLBACK_0(BeginInteract)
	EXPOSE_CALLBACK_0(EndInteract)
};
