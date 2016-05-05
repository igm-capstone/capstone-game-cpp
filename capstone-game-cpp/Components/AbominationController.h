#pragma once
#include "MinionController.h"

class AbominationController : public MinionController
{
	friend class Factory<AbominationController>;

	AbominationController();
	~AbominationController();

public:
};

