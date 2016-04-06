#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class StaticMesh : public BaseSceneObject
{
	friend class Factory<StaticMesh>;

public:
	class OrientedBoxColliderComponent* mColliderComponent;

private:
	StaticMesh() : mColliderComponent(Factory<OrientedBoxColliderComponent>::Create())
	{
		mColliderComponent->mIsDynamic = false;
		mColliderComponent->mSceneObject = this;
	};
	~StaticMesh() {};
};