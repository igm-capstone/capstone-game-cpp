#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class StaticMesh : public BaseSceneObject
{
	friend class Factory<StaticMesh>;

public:
	class BoxColliderComponent* mColliderComponent;

private:
	StaticMesh() : mColliderComponent(Factory<BoxColliderComponent>::Create()) {};
	~StaticMesh() {};
};