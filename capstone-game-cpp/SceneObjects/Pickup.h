#pragma once
#include "BaseSceneObject.h"

class Pickup : public BaseSceneObject
{
	friend class Factory<Pickup>;

public:
	std::string mSkill;

private:
	Pickup() : mSkill("") {}
	~Pickup() {}
};

