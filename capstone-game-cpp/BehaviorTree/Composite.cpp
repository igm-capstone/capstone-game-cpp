#include "stdafx.h"
#include "Composite.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Composite::Composite(Tree& tree, std::string name) : Behavior(tree, name)
{
	mOnIMGUI = [&](int level)
	{
		if (ImGui::TreeNode((void*)(intptr_t)0, "Minion"))
		{
			for (auto child : mChildren)
			{
				child->DumpIMGUI(level + 1);
			}
			ImGui::TreePop();
		}
	};
}

void Composite::Add(Behavior& behavior)
{
	mChildren.push_back(&behavior);
}
