#pragma once
#include "BaseComponent.h"

class BaseSceneObject;

class GhostController : public BaseComponent
{
	friend class Factory<GhostController>;

private:
	GhostController();
	~GhostController();

public:
	static void DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
};

