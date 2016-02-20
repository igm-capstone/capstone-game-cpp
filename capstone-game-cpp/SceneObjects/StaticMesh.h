#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class StaticMesh : public BaseSceneObject
{

	friend class Factory<StaticMesh>;

private:
	StaticMesh() {};
	~StaticMesh() {};
};