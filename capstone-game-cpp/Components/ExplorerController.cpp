#include <stdafx.h>
#include "ExplorerController.h"
#include <SceneObjects/Explorer.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Intersection.h>
#include <trace.h>
#include <Colors.h>
#include <Mathf.h>

using namespace cliqCity::graphicsMath;

ExplorerController::ExplorerController() :
	mInput((&Singleton<Engine>::SharedInstance())->GetInput()), 
	mApplication(&Application::SharedInstance()),
	mSprintDuration(0),
	mAcceleration(10.0f), 
	mBaseMoveSpeed(20.0f), 
	mSpeedMultiplier(1), 
	mCurrentSpeed(0), 
	mSpeed(0.01f),
	mIsInteracting(false)
{

}

ExplorerController::~ExplorerController()
{
	
}

bool ExplorerController::Move(float dt, vec3f& pos)
{
	// update speed multiplier for the sprint skill
	mSpeedMultiplier = mSprintDuration ? 2.0f : 1.0f;

	if (mSprintDuration > 0)
	{
		mSprintDuration -= min(dt, mSprintDuration);
	}

	// vertical and horizontal speed components
	float hSpeed = (mInput->GetKey(KEYCODE_LEFT) ? -1.0f : 0.0f) + (mInput->GetKey(KEYCODE_RIGHT) ? 1.0f : 0.0f);
	float vSpeed = (mInput->GetKey(KEYCODE_DOWN) ? -1.0f : 0.0f) + (mInput->GetKey(KEYCODE_UP) ? 1.0f : 0.0f);

	bool wantToMove = hSpeed || vSpeed;

	vec3f dirVector = wantToMove ? normalize(vec2f(hSpeed, vSpeed)) : vec3f();

	// speed per second
	vec3f targetSpeed = wantToMove ? mBaseMoveSpeed * mSpeedMultiplier * dirVector : vec3f();
	mCurrentSpeed = Mathf::Lerp(mCurrentSpeed, targetSpeed, mAcceleration * dt);

	// delta space for the current frame
	vec3f ds = mCurrentSpeed * dt;

	if (magnitude(ds) > 0.001f) {
		pos += ds;
		return true;
	}
	return false;
}

bool ExplorerController::Rotate(float dt, vec3f& pos, quatf& rot)
{
	// rotate towards mouse
	auto mousePosition = mApplication->mGroundMousePosition;
	mousePosition.z = 0;

	TRACE_LINE(pos, mousePosition, Colors::red);

	auto dir = mousePosition - pos;
	quatf newRot = normalize(quatf::angleAxis(atan2(dir.y, dir.x), vec3f(0, 0, 1)) * mModelRotation);

	auto hasRotated = rot == newRot;
	rot = newRot;

	return hasRotated;
}

void ExplorerController::UpdateInteractWill()
{
	if (mInput->GetKeyDown(KEYCODE_OEM_PERIOD))
	{
		mIsInteracting = true;
	}
	else if (mInput->GetKeyUp(KEYCODE_OEM_PERIOD))
	{
		mIsInteracting = false;
	}
}

// dt in milliseconds
bool ExplorerController::Update(double milliseconds)
{
	if (!mIsActive) return false;

	// delta time in seconds
	float dt = float(milliseconds) * 0.001f;

	auto pos = mSceneObject->mTransform->GetPosition();
	auto rot = mSceneObject->mTransform->GetRotation();

	bool hasMoved   = Move(dt, pos);
	bool hasRotated = Rotate(dt, pos, rot);

	if (hasMoved /*|| hasRotated*/)
	{
		OnMove(pos, rot);
	}

	UpdateInteractWill();

	return hasMoved || hasRotated;
}

void ExplorerController::Sprint(float duration)
{
	mSprintDuration = duration;
}

void ExplorerController::SetBaseRotation(const float& x, const float& y, const float& z)
{
	mModelRotation = quatf::rollPitchYaw(z, x, y);
}
