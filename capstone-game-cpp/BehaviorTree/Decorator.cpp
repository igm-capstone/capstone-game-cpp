#include "stdafx.h"
#include "Decorator.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Decorator::Decorator(Tree& tree, Behavior& child, std::string name)
	: Behavior(tree, name)
	, mChild(child)
{

	mOnIMGUI = [&](int level)
	{
		if (ImGui::TreeNode((void*)(intptr_t)0, "Minion"))
		{
			mChild.DumpIMGUI(level + 1);
			ImGui::TreePop();
		}
	};
}
