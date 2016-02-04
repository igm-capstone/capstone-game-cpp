#pragma once
#include "BaseComponent.h"

class Skill;

class ExplorerController : public BaseComponent
{
private:
	Input* mInput;
	double mSprintDuration;

public:
	float mSpeed;

	ExplorerController();
	~ExplorerController();

	bool Update(double milliseconds);

	static void DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);

	EXPOSE_CALLBACK(Move, vec3f)
};
