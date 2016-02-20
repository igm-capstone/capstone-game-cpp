#pragma once
#include "BaseComponent.h"
#include <ScareTacticsApplication.h>

class BaseSceneObject;

class GhostController : public BaseComponent
{
	friend class Factory<GhostController>;

public:

	void Update(double milliseconds);

private:
	
	Application* mApplication;
	class Input* mInput;

	GhostController();
	~GhostController();
};

