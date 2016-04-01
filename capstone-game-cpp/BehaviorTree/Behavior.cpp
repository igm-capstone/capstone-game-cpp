#include "stdafx.h"
#include "Behavior.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

using namespace BehaviorTree;


Behavior::Behavior(Tree& tree, std::string name) :
	mTree(tree),
	mName(name),
	mStatus(BehaviorStatus::Invalid),
	mObserver(BehaviorObserver::Default()),
	mOnUpdate(nullptr),
	mOnReset(nullptr),
	mOnInitialize(nullptr),
	mOnTerminate(nullptr)
{
	mOnIMGUI = [&](int& id, int level)
	{
		BeginIMGUI();

		std::stringstream ss;
		ss << "[" << mStatus << "] " << mName;
		if (ImGui::TreeNode(reinterpret_cast<void*>(intptr_t(id)), ss.str().c_str()))
		{
			ImGui::TreePop();
		}

		EndIMGUI();
	};
}

Behavior::~Behavior()
{
}

BehaviorStatus Behavior::Tick(void* userData)
{
	if (mOnInitialize && mStatus == BehaviorStatus::Invalid)
	{
		mOnInitialize(*this, userData);
	}

	if (mOnUpdate)
	{
		mStatus = mOnUpdate(*this, userData);
	}

	if (mOnTerminate && mStatus != BehaviorStatus::Running)
	{
		mOnTerminate(*this, userData, mStatus);
	}

	return mStatus;
}

void Behavior::Reset(void* data)
{
	if (mOnReset && mStatus != BehaviorStatus::Invalid)
	{
		mOnReset(*this, data);
	}
}
