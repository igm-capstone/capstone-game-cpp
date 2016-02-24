#include "stdafx.h"
#include "Composite.h"

void Composite::Add(Behavior& behavior)
{
	mChildren.push_back(&behavior);
}
