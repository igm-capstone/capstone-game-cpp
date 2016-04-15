#include "stdafx.h"
#include "GhostController.h"
#include <SceneObjects/Ghost.h>
#include <CameraManager.h>
#include <Rig3D/Intersection.h>
#include <Mathf.h>
#include <Colors.h>


void GhostController::Update(double milliseconds)
{
	if (!mIsActive) return;

	auto ghost = static_cast<Ghost*>(mSceneObject);
	auto camManager = ghost->mCameraManager;

	if (mInput->GetMouseButton(MOUSEBUTTON_RIGHT))
	{
		// calculate ds (delta space) for the mouse
		vec2f lastScreenPos = mInput->prevMousePosition;
		//vec2f currScreenPos = mInput->mousePosition;
		vec3f lastWorldPos  = camManager->Screen2WorldAtZ0(lastScreenPos);
		vec3f currWorldPos  = Application::SharedInstance().mGroundMousePosition; //camManager->Screen2WorldAtZ0(currScreenPos);
		vec3f ds = lastWorldPos - currWorldPos;

		// get camera projected bounds (only valid for orthogonal camera)
		//vec3f origin = camManager->Viewport2WorldAtZ0({ 0.0f, 0.0f });
		//vec3f extents = camManager->Viewport2WorldAtZ0({ 0.5f, 0.5f }) - origin;
		vec3f origin = camManager->Screen2WorldAtZ0({ 800, 500 });
		vec3f extents = camManager->Screen2WorldAtZ0({ 800 + 600, 500 - 300 }) - origin;

		auto sceneBounds = mApplication->GetCurrentScene()->mFloorCollider;

		// we want to test if scene bounds will contain the camera frame 
		// after the movement. If not, we need to remove the excess offset
		// from ds.
		AABB<vec2f> targetFrame = { origin + ds, extents }; 
		if (!ContainsAABB(targetFrame, sceneBounds))
		{
			vec2f targetOrigin = targetFrame.origin;

			// move frame bounds inside scene bounds
			targetFrame = Mathf::Fit(targetFrame, sceneBounds); 

			vec2f overflow = targetOrigin - targetFrame.origin;
			ds -= overflow;
		}

		TRACE_LINE(origin + extents, origin - extents, Colors::blue);
		TRACE_LINE(targetFrame.origin + targetFrame.halfSize, targetFrame.origin - targetFrame.halfSize, Colors::red);
		TRACE_LINE(sceneBounds.origin + sceneBounds.halfSize, sceneBounds.origin - sceneBounds.halfSize, Colors::green);

		camManager->ChangeLookAtBy(ds);
	}


	if (mInput->GetKeyDown(KEYCODE_1))
	{
		ghost->SetActiveSkill(1);
	} 
	else if (mInput->GetKeyDown(KEYCODE_2))
	{
		ghost->SetActiveSkill(2);
	}
	else if (mInput->GetKeyDown(KEYCODE_3))
	{
		ghost->SetActiveSkill(3);
	}
	else if (mInput->GetKeyDown(KEYCODE_4))
	{
		ghost->SetActiveSkill(4);
	}

	ghost->TickMana(float(milliseconds));
}

GhostController::GhostController(): 
	mInput(Singleton<Engine>::SharedInstance().GetInput())
{
	mApplication = &Application::SharedInstance();
}


GhostController::~GhostController()
{
}



