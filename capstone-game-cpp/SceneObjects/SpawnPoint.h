#pragma once
#pragma once
#include "BaseSceneObject.h"
#include <Pool.h>

class SpawnPoint : public BaseSceneObject
{
	friend class Factory<SpawnPoint>;

public:

private:
	SpawnPoint() {}
	~SpawnPoint() {};
};