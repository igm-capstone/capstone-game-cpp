#include "stdafx.h"
#include "Tree.h"
#include <trace.h>
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Tree::Tree(std::string name) : Behavior(*this, name), mRootBehavior(nullptr)
{
	SetUpdateCallback(OnUpdate);

	mOnIMGUI = [&](int level)
	{
		if (ImGui::TreeNode((void*)(intptr_t)0, "Minion"))
		{
			mRootBehavior->DumpIMGUI(level + 1);
			ImGui::TreePop();
		}
	};
}

void Tree::Start(Behavior& bh)
{
	mRootBehavior = &bh;
}

BehaviorStatus Tree::OnUpdate(Behavior& bh, void* userData)
{
	auto& self = reinterpret_cast<Tree&>(bh);
	
	return self.mRootBehavior->Tick(userData);
}