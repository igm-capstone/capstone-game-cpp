#include "stdafx.h"
#include "CameraManager.h"
#include <Rig3D/Visibility.h>
#include <Rig3D/Intersection.h>
#include "RayCast.h"
#include "Mathf.h"

CameraManager::CameraManager()
{
	mRenderer = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetRenderer();
	MoveCamera(vec3f(0,0,0), vec3f(0,0,-100)); //default camera

	pPlane.normal = vec3f(0, 0, 1);
	pPlane.distance = 0;

	mIsOrto = false;
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

void CameraManager::SetLevelBounds(vec2f center, vec2f extends)
{
	mLevelCenter = center;
	mLevelExtends = extends;

	mCameraPersp.SetViewMatrix(mat4f::lookAtLH(center, center, vec3f(0, 1, 0)));
	mCBufferFullLevelOrto.view = mCBufferPersp.view;

	mCameraQuadOrto.SetProjectionMatrix(mat4f::normalizedOrthographicLH(mLevelCenter.x - mLevelExtends.x, mLevelCenter.x + mLevelExtends.x, mLevelCenter.y - mLevelExtends.y, mLevelCenter.y + mLevelExtends.y, mNearPlane, mFarPlane));
	mCBufferFullLevelOrto.projection = mCameraQuadOrto.GetProjectionMatrix().transpose();
}

void CameraManager::MoveCamera(const vec3f& lookAt, const vec3f& origin)
{
	mLookAt = lookAt;
	mOrigin = origin;
	
	mCameraPersp.SetViewMatrix(mat4f::lookAtLH(lookAt, origin, vec3f(0, 1, 0)));
	mCBufferPersp.view = mCameraPersp.GetViewMatrix().transpose();
	
	mCameraScreenOrto.SetViewMatrix(mCameraPersp.GetViewMatrix());
	mCBufferScreenOrto.view = mCBufferPersp.view;

	OnResize();
}

void CameraManager::OnResize()
{
	auto aspectRatio = mRenderer->GetAspectRatio();

	mCameraPersp.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(mFOV, aspectRatio, mNearPlane, mFarPlane));
	mCBufferPersp.projection = mCameraPersp.GetProjectionMatrix().transpose();

	mCameraScreenOrto.SetProjectionMatrix(mat4f::normalizedOrthographicLH(0, float(mRenderer->GetWindowWidth()), float(mRenderer->GetWindowHeight()), 0, mNearPlane, mFarPlane));
	mCBufferScreenOrto.projection = mCameraScreenOrto.GetProjectionMatrix().transpose();
}

vec2f CameraManager::World2Screen(const vec3f& world)
{
	mat4f viewProjectionMatrix = mCameraPersp.GetViewMatrix() * mCameraPersp.GetProjectionMatrix();

	auto ndc = vec4f(world,1) * viewProjectionMatrix;
 	ndc /= ndc.w;

	float winX = round(((ndc.x + 1) / 2.0f) * mRenderer->GetWindowWidth());
	float winY = round(((1 - ndc.y) / 2.0f) * mRenderer->GetWindowHeight());

	return vec2f(winX, winY);
}

vec2f CameraManager::Screen2Viewport(const vec2f& screen)
{
	return {
		2.0f * screen.x / mRenderer->GetWindowWidth() - 1,
		1 - 2.0f * screen.y / mRenderer->GetWindowHeight()
	};
}

vec3f CameraManager::Screen2WorldAtZ0(const vec2f& screen)
{
	vec2f viewport = Screen2Viewport(screen);
	return Viewport2WorldAt_Unclamped(viewport, 0);
}

vec3f CameraManager::Viewport2WorldAtZ0(const vec2f& viewport)
{
	return Viewport2WorldAt(viewport, 0);
}

vec3f CameraManager::Screen2WorldAt(const vec2f& screen, float z)
{
	vec2f viewport = Screen2Viewport(screen);
	return Viewport2WorldAt_Unclamped(viewport, z);
}

vec3f CameraManager::Viewport2WorldAt(const vec2f& viewport, float z)
{
	vec2f clamped = Mathf::Clamp(viewport, vec2f(-1, -1), vec2f(1, 1));
	return Viewport2WorldAt_Unclamped(clamped, z);
}

vec3f CameraManager::Viewport2WorldAt_Unclamped(const vec2f& viewport, float z)
{
	Viewport2Ray(viewport);

	vec3f world;
	pPlane.distance = z;
	IntersectRayPlane(pRay, pPlane, world, pDist);
	return world;
}

Ray<vec3f> CameraManager::Screen2Ray(const vec2f& screen)
{
	vec2f viewport = Screen2Viewport(screen);

	return Viewport2Ray(viewport);
}

Ray<vec3f> CameraManager::Viewport2Ray(const vec2f& viewport)
{
	vec4f ray_clip = vec4f(viewport.x, viewport.y, -1.0f, 1.0f) * (mCameraPersp.GetProjectionMatrix()).inverse();
	ray_clip.w = 0;

	pRay.normal = ray_clip * (mCameraPersp.GetViewMatrix()).inverse();
	pRay.origin = mOrigin;

	return pRay;
}


