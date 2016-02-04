#include <stdafx.h>
#include "ExplorerController.h"
#include <SceneObjects/Explorer.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Intersection.h>
#include <trace.h>

ExplorerController::ExplorerController() : mSpeed(0.01f)
{
	mInput = (&Singleton<Engine>::SharedInstance())->GetInput();
}

ExplorerController::~ExplorerController()
{
	
}

bool ExplorerController::Update()
{
	if (!mIsActive) return false;

	bool hasMoved = false;

	auto pos = mSceneObject->mTransform->GetPosition();
	if (mInput->GetKey(KEYCODE_LEFT))
	{
		pos.x -= mSpeed;
		hasMoved = true;
	}
	if (mInput->GetKey(KEYCODE_RIGHT))
	{
		pos.x += mSpeed;
		hasMoved = true;
	}
	if (mInput->GetKey(KEYCODE_UP))
	{
		pos.y += mSpeed;
		hasMoved = true;
	}
	if (mInput->GetKey(KEYCODE_DOWN))
	{
		pos.y -= mSpeed;
		hasMoved = true;
	}

	if (hasMoved && mOnControllerMove)
		mOnControllerMove(mSceneObject, pos);

	return hasMoved;
}

void ExplorerController::DoSprint(Skill* skill, BaseSceneObject* target, vec3f worldPosition)
{
	TRACE_LOG("Sprint!!");
}