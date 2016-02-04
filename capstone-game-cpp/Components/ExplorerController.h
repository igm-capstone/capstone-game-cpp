#pragma once
#include "BaseComponent.h"

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
};
