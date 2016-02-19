#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class StaticMesh : public BaseSceneObject
{

	friend class Factory<StaticMesh>;

public:
	class IMesh* mMesh;

private:
	StaticMesh() : mMesh(nullptr) {};
	~StaticMesh() {};
};