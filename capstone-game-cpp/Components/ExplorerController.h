#pragma once
#include "BaseComponent.h"

class ExplorerController : public BaseComponent
{
public:
	float mSpeed;
	
	ExplorerController();
	~ExplorerController();

	bool Move();
};
