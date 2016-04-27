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
	
	float mAngle;
	float mMouseLock;

	float mSprintDuration;
	quatf mModelRotation;

	float mSpeedMultiplier;
	float mAcceleration;
	vec3f mCurrentSpeed;
	void UpdateInteractWill();
	
	// delta time in seconds
	bool Move(float dt, vec3f& pos);
	bool RotateTowardsMoveDirection(float dt, vec3f& pos, quatf& rot);
	bool RotateTowardsMousePosition(float dt, vec3f& pos, quatf& rot);
	bool UpdateRotation(float angle, quatf & rot);
	bool CanMove();

public:
	class AnimationController* mAnimationController;
	float mBaseMoveSpeed;
	float mSprintMultiplier;

	bool mIsInteracting;
	void ConsumeInteractWill();

	bool Update(double milliseconds);
	void Sprint(float duration);
	void Melee();

	void PlayStateAnimation(AnimationControllerState state);
	void PauseStateAnimation(AnimationControllerState state);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
	EXPOSE_CALLBACK_0(BeginInteract)
	EXPOSE_CALLBACK_0(EndInteract)
};
