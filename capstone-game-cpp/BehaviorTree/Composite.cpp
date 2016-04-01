#include "stdafx.h"
#include "Composite.h"

using namespace BehaviorTree;

Composite::Composite(Tree& tree, std::string name) : Behavior(tree, name)
{
	mDumpCallback = [&](std::stringstream& ss, int level)
	{
		ss << "[" << mStatus << "] " << std::string(level, '\t') << mName << std::endl;
		for (auto child : mChildren)
		{
			child->Dump(ss, level + 1);
		}
	};
}

void Composite::Add(Behavior& behavior)
{
	mChildren.push_back(&behavior);
}
