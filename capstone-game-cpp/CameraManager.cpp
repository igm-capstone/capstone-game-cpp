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

	mCameraPersp.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(0.25f * PI, aspectRatio, 0.1f, 1000.0f));
	mCBufferPersp.projection = mCameraPersp.GetProjectionMatrix().transpose();

	//Get matching orto projection - needs math review
	Rig3D::Frustum me;
	auto mv = mCameraPersp.GetViewMatrix() * mCameraPersp.GetProjectionMatrix();
	ExtractNormalizedFrustumLH(&me, mv);

	mCameraOrto.SetProjectionMatrix(mat4f::normalizedOrthographicLH(me.left.distance / me.left.normal.x, me.right.distance / me.right.normal.x, me.bottom.distance / me.bottom.normal.y, me.top.distance / me.top.normal.y, 0.1f, 1000.0f));
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

	int winX = round(((ndc.x + 1) / 2.0) * mRenderer->GetWindowWidth());
	int winY = round(((1 - ndc.y) / 2.0) * mRenderer->GetWindowHeight());

	return vec2f(winX, winY);
}

vec3f CameraManager::Screen2WorldAt0(vec2f screen)
{
	Screen2Ray(screen);

	vec3f world;
	IntersectRayPlane(pRay, pPlane, world, pDist);
	return world;
}

Ray<vec3f> CameraManager::Screen2Ray(vec2f screen)
{
	double x = 2.0 * screen.x / mRenderer->GetWindowWidth() - 1;
	double y = 1 - 2.0 * screen.y / mRenderer->GetWindowHeight();

	vec4f ray_clip = vec4f(x, y, -1, 1) * (mCameraPersp.GetProjectionMatrix()).inverse();
	ray_clip.w = 0;

	pRay.normal = ray_clip * (mCameraPersp.GetViewMatrix()).inverse();
	pRay.origin = mOrigin;

	return pRay;
}