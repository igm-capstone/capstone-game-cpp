#pragma once
#include "MinionController.h"

class ImpController : public MinionController
{
	friend class Factory<ImpController>;

	ImpController();
	~ImpController();

public:
};

