#pragma once
#include "BaseComponent.h"
#include <ScareTacticsApplication.h>

class Skill;

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

public:
	float mSpeed;

	bool Update(double milliseconds);
	void Sprint(float duration);
	void SetBaseRotation(const float& x, const float& y, const float& z);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
};
