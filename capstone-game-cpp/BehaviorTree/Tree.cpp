#include "stdafx.h"
#include "Tree.h"
#include <trace.h>
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Tree::Tree(std::string name) : Behavior(*this, name), mRootBehavior(nullptr)
{
	SetUpdateCallback(OnUpdate);

	mOnIMGUI = [&](int& id, int level)
	{
		BeginIMGUI();

		std::stringstream ss;
		ss << "[" << mStatus << "] " << mName;

		ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Appearing);
		if (ImGui::TreeNode(reinterpret_cast<void*>(intptr_t(++id)), ss.str().c_str()))
		{
			mRootBehavior->DumpIMGUI(id, level + 1);
			ImGui::TreePop();
		}

		EndIMGUI();
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