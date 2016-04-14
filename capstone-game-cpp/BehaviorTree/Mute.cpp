#include "stdafx.h"
#include "Mute.h"
#include "Tree.h"

using namespace BehaviorTree;

Mute::Mute(Tree& tree, Behavior* child, std::string name)
	: Decorator(tree, child, name)
{
	SetUpdateCallback(&OnUpdate);
}

BehaviorStatus Mute::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Mute&>(bh);

	self.mChild->Tick(data);

	return BehaviorStatus::Success;
}