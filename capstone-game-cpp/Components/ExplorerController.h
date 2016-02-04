#pragma once
#include "BaseComponent.h"

class Skill;

class ExplorerController : public BaseComponent
{
private:
	Input* mInput;

public:
	float mSpeed;

	ExplorerController();
	~ExplorerController();

	bool Update();

	static void DoSprint(Skill* skill, BaseSceneObject* target, vec3f worldPosition);

	EXPOSE_CALLBACK(Move, vec3f)
};
