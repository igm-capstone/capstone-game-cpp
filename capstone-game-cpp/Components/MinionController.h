#pragma once
#include "BaseComponent.h"

class MinionController : public BaseComponent
{
	friend class Factory<MinionController>;

	MinionController();
	~MinionController();

public:
	float mSpeed;

	bool Update(double milliseconds);

	EXPOSE_CALLBACK_1(Move, vec3f)
};
