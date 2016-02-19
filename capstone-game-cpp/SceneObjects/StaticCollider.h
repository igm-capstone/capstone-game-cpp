#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class StaticCollider : public BaseSceneObject
{

	friend class Factory<StaticCollider>;

public:
	class OrientedBoxColliderComponent* mBoxCollider;

private:
	StaticCollider() : mBoxCollider(Factory<OrientedBoxColliderComponent>::Create()) {};
	~StaticCollider() {};
};