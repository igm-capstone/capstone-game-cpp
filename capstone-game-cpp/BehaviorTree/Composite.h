﻿#pragma once
#include "Behavior.h"

class Composite : public Behavior
{
public:
	std::vector<Behavior*> mChildren;
};

template <class COMPOSITE>
class MockComposite : public COMPOSITE
{
public:
	MockComposite(class BehaviorTree& bt, size_t size)
		: COMPOSITE(bt)
	{
		for (size_t i = 0; i<size; ++i)
		{
			COMPOSITE::mChildren.push_back(new MockBehavior);
		}
	}

	~MockComposite()
	{
		for (size_t i = 0; i<COMPOSITE::mChildren.size(); ++i)
		{
			delete COMPOSITE::mChildren[i];
		}
	}

	MockBehavior& operator[](size_t index)
	{
		ASSERT(index < COMPOSITE::mChildren.size());
		return *static_cast<MockBehavior*>(COMPOSITE::mChildren[index]);
	}
};
