#include "stdafx.h"
#include "CameraManager.h"
#include <Rig3D/Visibility.h>
#include <Rig3D/Intersection.h>
#include "RayCast.h"

CameraManager::CameraManager()
{
	mRenderer = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetRenderer();
	MoveCamera(vec3f(0,0,0), vec3f(0,0,-100)); //default camera

	pPlane.normal = vec3f(0, 0, -1);
	pPlane.distance = 0;
}

CameraManager::~CameraManager()
{
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
	
	mCameraPersp.SetViewMatrix(mat4f::lookAtLH(lookAt, origin, vec3f(0, 1, 0)));
	mCBufferPersp.view = mCameraPersp.GetViewMatrix().transpose();
	
	mCameraOrto.SetViewMatrix(mCameraPersp.GetViewMatrix());
	mCBufferOrto.view = mCBufferPersp.view;

	OnResize();
}

void CameraManager::OnResize()
{
	auto aspectRatio = mRenderer->GetAspectRatio();

	mCameraPersp.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(mFOV, aspectRatio, mNearPlane, mFarPlane));
	mCBufferPersp.projection = mCameraPersp.GetProjectionMatrix().transpose();

	mCameraOrto.SetProjectionMatrix(mat4f::normalizedOrthographicLH(0, float(mRenderer->GetWindowWidth()), float(mRenderer->GetWindowHeight()), 0, mNearPlane, mFarPlane));
	mCBufferOrto.projection = mCameraOrto.GetProjectionMatrix().transpose();
}

vec3f CameraManager::GetForward()
{
	if (true)
	{
		mForward = vec3f(0, 0, 1) * mat3f(mCBufferPersp.view);
	}

	return mForward;
}

vec2f CameraManager::World2Screen(vec3f world)
{
	mat4f viewProjectionMatrix = mCameraPersp.GetViewMatrix() * mCameraPersp.GetProjectionMatrix();

	auto ndc = vec4f(world,1) * viewProjectionMatrix;
 	ndc /= ndc.w;

	float winX = round(((ndc.x + 1) / 2.0f) * mRenderer->GetWindowWidth());
	float winY = round(((1 - ndc.y) / 2.0f) * mRenderer->GetWindowHeight());

	return vec2f(winX, winY);
}

vec3f CameraManager::Screen2WorldAtZ0(vec2f screen)
{
	Screen2Ray(screen);

	vec3f world;
	IntersectRayPlane(pRay, pPlane, world, pDist);
	return world;
}

Ray<vec3f> CameraManager::Screen2Ray(vec2f screen)
{
	float x = 2.0f * screen.x / mRenderer->GetWindowWidth() - 1;
	float y = 1 - 2.0f * screen.y / mRenderer->GetWindowHeight();

	vec4f ray_clip = vec4f(x, y, -1.0f, 1.0f) * (mCameraPersp.GetProjectionMatrix()).inverse();
	ray_clip.w = 0;

	pRay.normal = ray_clip * (mCameraPersp.GetViewMatrix()).inverse();
	pRay.origin = mOrigin;

	return pRay;
}