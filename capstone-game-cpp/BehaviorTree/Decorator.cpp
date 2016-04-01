#include "stdafx.h"
#include "Decorator.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;

Decorator::Decorator(Tree& tree, Behavior& child, std::string name)
	: Behavior(tree, name)
	, mChild(child)
{

	mOnIMGUI = [&](int& id, int level)
	{
		BeginIMGUI();

		std::stringstream ss;
		ss << "[" << mStatus << "] " << mName;
		if (ImGui::TreeNode(reinterpret_cast<void*>(intptr_t(id)), ss.str().c_str()))
		{
			mChild.DumpIMGUI(++id, level + 1);
			ImGui::TreePop();
		}

		EndIMGUI();
	};
}
