#pragma once
#include "BaseComponent.h"

class Skill;

class ExplorerController : public BaseComponent
{
private:
	Input* mInput;
	typedef void(*MoveCallback)(BaseSceneObject*, vec3f);

private:
	//callbacks
	MoveCallback mOnControllerMove;


public:
	void RegisterCallback(MoveCallback handler)
	{
		mOnControllerMove = handler;
	}

public:
	float mSpeed;

	ExplorerController();
	~ExplorerController();

	bool Update();

	static void DoSprint(Skill* skill, BaseSceneObject* target, vec3f worldPosition);
};
