#include "stdafx.h"
#include "Composite.h"

Composite::Composite(std::string name): Behavior(name)
{
}

void Composite::Add(Behavior& behavior)
{
	mChildren.push_back(&behavior);
}
