#include "stdafx.h"
#include "CameraManager.h"
#include <Rig3D/Visibility.h>


CameraManager::CameraManager()
{
}


CameraManager::~CameraManager()
{
}

void CameraManager::Initialize(const float aspectRatio)
{
	mAspectRatio = aspectRatio;
	MoveCamera(vec3f(0,0,0), vec3f(0,0,-100)); //default camera
}

void CameraManager::ChangeLookAtTo(const vec3f& newLookAt)
{
	MoveCamera(newLookAt, mOrigin - mLookAt + newLookAt);
}

void CameraManager::ChangeLookAtBy(const vec3f& offset)
{
	MoveCamera(mLookAt + offset, mOrigin + offset);
}

void CameraManager::MoveCamera(const vec3f& lookAt, const vec3f& origin)
{
	mLookAt = lookAt;
	mOrigin = origin;
	mCameraPersp.view = mat4f::lookAtLH(lookAt, origin, vec3f(0, 1, 0)).transpose();
	mCameraOrto.view = mCameraPersp.view;
	ChangeAspectRatio(mAspectRatio);
}

void CameraManager::ChangeAspectRatio(const float aspectRatio)
{
	mAspectRatio = aspectRatio;

	mCameraPersp.projection = mat4f::normalizedPerspectiveLH(0.25f * PI, aspectRatio, 0.1f, 1000.0f).transpose();

	//Get matching orto camera - needs math review
	Rig3D::Frustum me;
	auto mv = mCameraPersp.view.transpose() * mCameraPersp.projection.transpose();
	ExtractNormalizedFrustumLH(&me, mv);
	mCameraOrto.projection = mat4f::normalizedOrthographicLH(me.left.distance / me.left.normal.x, me.right.distance / me.right.normal.x, me.bottom.distance / me.bottom.normal.y, me.top.distance / me.top.normal.y, 0.1f, 1000.0f).transpose();
}

vec3f CameraManager::GetForward()
{
	if (true)
	{
		mForward = vec3f(0, 0, 1) * mat3f(mCameraPersp.view);
	}

	return mForward;
}