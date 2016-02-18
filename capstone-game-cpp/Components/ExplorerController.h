#pragma once
#include "BaseComponent.h"
#include <ScareTacticsApplication.h>

class ExplorerController : public BaseComponent
{
	friend class Factory<ExplorerController>;

	ExplorerController();
	~ExplorerController();

	Input* mInput;
	Application* mApplication;
	double mSprintDuration;
	quatf mModelRotation;

	float mAcceleration;
	float mBaseMoveSpeed;
	float mSpeedMultiplier;
	vec3f mCurrentSpeed;

	// delta time in seconds
	bool Move(float dt, vec3f& pos);
	bool Rotate(float dt, vec3f& pos, quatf& rot);
	void UpdateInteractWill();

public:
	float mSpeed;
	bool mIsInteracting;

	bool Update(double milliseconds);
	void Sprint(float duration);
	void SetBaseRotation(const float& x, const float& y, const float& z);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
	EXPOSE_CALLBACK_0(BeginInteract)
	EXPOSE_CALLBACK_0(EndInteract)
};
