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
	mOnIMGUI = [&](int level)
	{
		if (ImGui::TreeNode((void*)(intptr_t)0, "Minion"))
		{
			ImGui::TreePop();
		}
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
