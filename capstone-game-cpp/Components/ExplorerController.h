#pragma once
#include "BaseComponent.h"

class ExplorerController : public BaseComponent
{
public:
	float mSpeed = 0.25f;
	
	bool Move();
};
