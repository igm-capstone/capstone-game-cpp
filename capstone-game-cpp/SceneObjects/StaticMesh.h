#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class StaticMesh : public BaseSceneObject
{
	friend class Factory<StaticMesh>;

	class BoxColliderComponent* mCullingBoxCollider;

private:
	StaticMesh() : mCullingBoxCollider(Factory<BoxColliderComponent>::Create()) {};
	~StaticMesh() {};
};