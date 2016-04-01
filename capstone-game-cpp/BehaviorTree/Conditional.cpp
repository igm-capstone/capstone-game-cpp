#include "stdafx.h"
#include "Conditional.h"
#include "Predicate.h"
#include "Tree.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Conditional::Conditional(Tree& tree, Behavior& child, Predicate& predicate, std::string name)
	: Decorator(tree, child, name)
	, mPredicate(predicate)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);

	mOnIMGUI = [&](int level)
	{
		if (ImGui::TreeNode((void*)(intptr_t)0, "Minion"))
		{
			mPredicate.DumpIMGUI(level + 1);
			mChild.DumpIMGUI(level + 1);
			ImGui::TreePop();
		}
	};
}

void Conditional::OnInitialize(Behavior& bh, void* data)
{
	//auto& self = static_cast<Conditional&>(bh);

	//self.mTree.Start(self.mChild, { &OnChildComplete, &self, data });
}

BehaviorStatus Conditional::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Conditional&>(bh);

	self.mPredicate.Tick(data);

	if (!self.mPredicate.IsValid())
	{
		return BehaviorStatus::Failure;
	}
	
	return self.mChild.Tick(data);
}