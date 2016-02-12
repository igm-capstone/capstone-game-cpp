#pragma once
#include "BaseComponent.h"

class Skill;

class ExplorerController : public BaseComponent
{
	friend class Factory<ExplorerController>;

	ExplorerController();
	~ExplorerController();

	Input* mInput;
	double mSprintDuration;

public:
	float mSpeed;

	bool Update(double milliseconds);

	static void DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	void SetBaseRotation(const float& x, const float& y, const float& z);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
};
