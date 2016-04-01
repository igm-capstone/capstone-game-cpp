#include "stdafx.h"
#include "Decorator.h"

using namespace BehaviorTree;

Decorator::Decorator(Tree& tree, Behavior& child, std::string name)
	: Behavior(tree, name)
	, mChild(child)
{
	mDumpCallback = [&](std::stringstream& ss, int level)
	{
		ss << "[" << mStatus << "] " << std::string(level, '\t') << mName << std::endl;
		mChild.Dump(ss, level + 1);
	};
}
