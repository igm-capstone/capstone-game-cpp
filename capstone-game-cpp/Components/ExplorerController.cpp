#include <stdafx.h>
#include "ExplorerController.h"
#include <Mathf.h>
#include <Colors.h>
#include <SceneObjects/Explorer.h>
#include <Components/AnimationUtility.h>
#include "Health.h"

using namespace cliqCity::graphicsMath;

#pragma region util

inline quatf CalculateExplorerRotation(float angle)
{
	return normalize(quatf::angleAxis(angle, vec3f(0, 0, 1)) * quatf::rollPitchYaw(-0.5f * PI, 0, 0) * quatf::rollPitchYaw(0, -0.5f * PI, 0));
}

#pragma endregion 

ExplorerController::ExplorerController() :
	mInput((&Singleton<Engine>::SharedInstance())->GetInput()),
	mApplication(&Application::SharedInstance()),
	mCameraManager(&Singleton<CameraManager>::SharedInstance()),
	mAngle(0),
	mMoveLockCooldown(0),
	mSprintDuration(0),
	mAcceleration(10.0f),
	mCurrentSpeed(0),
	mAnimationController(nullptr),
	mBaseMoveSpeed(20.0f),
	mSpeedMultiplier(1),
	mSprintMultiplier(1.5),
	mIsInteracting(false)
{

}

ExplorerController::~ExplorerController()
{
	
}

bool ExplorerController::Move(float dt, vec3f& pos)
{
	// update speed multiplier for the sprint skill
	mSpeedMultiplier = mSprintDuration ? mSprintMultiplier : 1.0f;

	if (mSprintDuration > 0)
	{
		mSprintDuration -= min(dt, mSprintDuration);
	}

	// vertical and horizontal speed components
	float hSpeed = (mInput->GetKey(KEYCODE_A) ? -1.0f : 0.0f) + (mInput->GetKey(KEYCODE_D) ? 1.0f : 0.0f);
	float vSpeed = (mInput->GetKey(KEYCODE_S) ? -1.0f : 0.0f) + (mInput->GetKey(KEYCODE_W) ? 1.0f : 0.0f);

	bool wantToMove = hSpeed || vSpeed;

	vec3f dirVector = wantToMove ? normalize(vec2f(hSpeed, vSpeed)) : vec3f();

	// speed per second
	vec3f targetSpeed = wantToMove ? mBaseMoveSpeed * mSpeedMultiplier * dirVector : vec3f();
	mCurrentSpeed = Mathf::Lerp(mCurrentSpeed, targetSpeed, mAcceleration * dt);

	// delta space for the current frame
	vec3f ds = mCurrentSpeed * dt * CanMove();

	if (magnitude(ds) > 0.001f) {
		pos += ds;
		return true;
	}
	return false;
}

bool ExplorerController::RotateTowardsMoveDirection(float dt, vec3f& pos, quatf& rot)
{
	auto lastPos = mSceneObject->mTransform->GetPosition();
	auto dir = normalize(pos - lastPos);

	if (magnitude(dir) == 0) {
		return false;
	}

	mTargetAngle = atan2f(dir.y, dir.x);
	return RotateTowardsTargetAngle(dt, rot);
}

bool ExplorerController::RotateTowardsMousePosition(float dt, vec3f& pos, quatf& rot)
{
	// rotate towards mouse

	auto mousePosition = mCameraManager->Screen2WorldAt(mInput->mousePosition, pos.z);
	auto lastPos = mSceneObject->mTransform->GetPosition();
	auto dir = normalize(mousePosition - lastPos);

	if (magnitude(dir) == 0) {
		return false;
	}

	mTargetAngle = atan2(dir.y, dir.x);
	return RotateTowardsTargetAngle(dt, rot);
}

bool ExplorerController::RotateTowardsTargetAngle(float dt, quatf& rot)
{
	mAngle = Mathf::LerpAngle(mAngle, mTargetAngle, 5 * dt);

	return UpdateRotation(mAngle, rot);
}

bool ExplorerController::UpdateRotation(float angle, quatf& rot) {
	quatf newRot = CalculateExplorerRotation(angle);

	auto hasRotated = rot != newRot;
	if (hasRotated/* && CanMove()*/)
	{
		rot = newRot;
	}

	return hasRotated;
}

void ExplorerController::UpdateInteractWill()
{
	if (mInput->GetKeyDown(KEYCODE_SPACE) || mInput->GetKeyDown(KEYCODE_OEM_PERIOD))
	{
		mIsInteracting = true;
	}
	else if (mInput->GetKeyUp(KEYCODE_SPACE) || mInput->GetKeyUp(KEYCODE_OEM_PERIOD))
	{
		mIsInteracting = false;
	}
}

void ExplorerController::ConsumeInteractWill()
{
	mIsInteracting = false;
}

// dt in milliseconds
bool ExplorerController::Update(double milliseconds)
{
	if (!mIsActive) return false;

	Explorer* pExplorer = reinterpret_cast<Explorer*>(mSceneObject);
	if (pExplorer->mHealth->GetHealth() <= 0)
	{
		return false;
	}

	// delta time in seconds
	float dt = float(milliseconds) * 0.001f;

	// this line is to prevent huge dts during debug time
	dt = min(dt, 0.05f);

	if (mMoveLockCooldown > 0) {
		mMoveLockCooldown = max(mMoveLockCooldown - dt, 0);
	}

	auto pos = mSceneObject->mTransform->GetPosition();
	auto rot = mSceneObject->mTransform->GetRotation();

	bool hasMoved   = Move(dt, pos);
	bool hasRotated = false;
	if (!CanMove()) {
		hasRotated = RotateTowardsTargetAngle(dt, rot);
	}
	else if (!hasMoved) {
		hasRotated = RotateTowardsMousePosition(dt, pos, rot);
	}
	else {
		hasRotated = RotateTowardsMoveDirection(dt, pos, rot);
	}

	if (hasMoved || hasRotated)
	{
		if (hasMoved)
		{
			PlayStateAnimation(ANIM_STATE_RUN);
		}
		else if (mAnimationController->GetState() != ANIM_STATE_MELEE)
		{
			PlayStateAnimation(ANIM_STATE_IDLE);
		}

		OnMove(pos, rot, hasMoved);
	}
	else
	{
		if (mAnimationController->GetState() != ANIM_STATE_MELEE)
		{
			PlayStateAnimation(ANIM_STATE_IDLE);
		}
	}

	UpdateInteractWill();

	return hasMoved || hasRotated;
}



void ExplorerController::Sprint(float duration)
{
	mSprintDuration = duration;
}

void ExplorerController::Melee()
{
	vec3f position = mSceneObject->mTransform->GetPosition();
	auto mousePosition = mCameraManager->Screen2WorldAt(mInput->mousePosition, position.z);
	auto lastPos = mSceneObject->mTransform->GetPosition();
	auto dir = normalize(mousePosition - lastPos);

	if (magnitude(dir) > 0.001f) 
	{
		mTargetAngle = atan2(dir.y, dir.x);
		mMoveLockCooldown = 1.0f;
	}

	PlayStateAnimation(ANIM_STATE_MELEE);
}

bool ExplorerController::CanMove()
{
	return mMoveLockCooldown <= 0;

	return mAnimationController->GetState() != ANIM_STATE_MELEE;
}

void ExplorerController::PlayStateAnimation(AnimationControllerState state)
{
	AnimationController* pAnimationController = reinterpret_cast<Explorer*>(mSceneObject)->mAnimationController;
	pAnimationController->SetState(state);
	pAnimationController->Resume();
}
void ExplorerController::PauseStateAnimation(AnimationControllerState state)
{
	AnimationController* pAnimationController = reinterpret_cast<Explorer*>(mSceneObject)->mAnimationController;
	if (pAnimationController->GetState() == state)
	{
		pAnimationController->Pause();
	}
}
