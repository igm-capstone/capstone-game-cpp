#include <stdafx.h>
#include "ExplorerController.h"
#include <SceneObjects/Explorer.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Intersection.h>
#include <trace.h>
#include <Colors.h>

ExplorerController::ExplorerController() :
	mInput((&Singleton<Engine>::SharedInstance())->GetInput()), 
	mApplication(&Application::SharedInstance()),
	mSprintDuration(0), 
	mSpeed(0.01f)
{

}

ExplorerController::~ExplorerController()
{
	
}

bool ExplorerController::Update(double milliseconds)
{
	if (!mIsActive) return false;

	bool hasMoved = false;

	float speed = mSpeed;

	if (mSprintDuration > 0)
	{
		speed *= 2;

		mSprintDuration -= min(milliseconds, mSprintDuration);
	}

	auto pos = mSceneObject->mTransform->GetPosition();
	if (mInput->GetKey(KEYCODE_LEFT))
	{
		pos.x -= speed;
		hasMoved = true;
	}
	if (mInput->GetKey(KEYCODE_RIGHT))
	{
		pos.x += speed;
		hasMoved = true;
	}
	if (mInput->GetKey(KEYCODE_UP))
	{
		pos.y += speed;
		hasMoved = true;
	}
	if (mInput->GetKey(KEYCODE_DOWN))
	{
		pos.y -= speed;
		hasMoved = true;
	}

	auto mousePosition = mApplication->mGroundMousePosition;
	mousePosition.z;

	TRACE_LINE(pos, mousePosition, Colors::red);

	auto dir = mousePosition - pos;

	auto rot = quatf::angleAxis(atan2(dir.y, dir.x), vec3f(0, 0, 1)) * mModelRotation;

	//if (hasMoved)
		OnMove(pos, rot);

	return hasMoved;
}

void ExplorerController::DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition)
{
	TRACE_LOG("Sprint!!");

	auto e = reinterpret_cast<Explorer*>(obj);
	e->mController->mSprintDuration = duration;
}

void ExplorerController::SetBaseRotation(const float& x, const float& y, const float& z)
{
	mModelRotation = quatf::rollPitchYaw(z, x, y);
}
