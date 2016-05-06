#pragma once
#pragma once
#include "BaseSceneObject.h"
#include "Factory.h"

class SpawnPoint : public BaseSceneObject
{
	friend class Factory<SpawnPoint>;

public:

private:
	SpawnPoint() {}
	~SpawnPoint() {};
};