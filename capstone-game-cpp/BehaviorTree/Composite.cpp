#include "stdafx.h"
#include "Composite.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Composite::Composite(Tree& tree, std::string name) : Behavior(tree, name)
{
	mOnIMGUI = [&](int& id, int level)
	{
		BeginIMGUI();

		std::stringstream ss;
		ss << "[" << mStatus << "] " << mName;
		if (ImGui::TreeNode(reinterpret_cast<void*>(intptr_t(id)), ss.str().c_str()))
		{
			for (auto child : mChildren)
			{
				child->DumpIMGUI(++id, level + 1);
			}
			ImGui::TreePop();
		}

		EndIMGUI();
	};
}

void Composite::Add(Behavior& behavior)
{
	mChildren.push_back(&behavior);
}
