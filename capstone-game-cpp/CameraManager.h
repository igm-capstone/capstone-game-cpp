#pragma once
#include "Uniforms.h"
#include <Rig3D/Graphics/Camera.h>
#include <Rig3D/Parametric.h>


class CameraManager
{
	Rig3D::Renderer* mRenderer;
	Rig3D::Camera mCameraOrto;
	Rig3D::Camera mCameraPersp;

	CBuffer::Camera	mCBufferOrto;
	CBuffer::Camera	mCBufferPersp;
	vec3f mForward;
	vec3f mOrigin;
	vec3f mLookAt;

	const float mFOV = 0.25f * PI;
	const float mNearPlane = 0.1f;
	const float mFarPlane = 200;

protected:
	CameraManager();
	~CameraManager();

public:

	vec3f GetOrigin() { return mOrigin; };
	vec3f GetLookAt() { return mLookAt; };

	CBuffer::Camera* GetCBufferPersp() { return &mCBufferPersp; }
	CBuffer::Camera* GetCBufferOrto() { return &mCBufferOrto; }


	void ChangeLookAtTo(const vec3f& newLookAt);
	void ChangeLookAtBy(const vec3f& offset);
	void MoveCamera(const vec3f& lookAt, const vec3f& origin);
	void OnResize();
	vec3f GetForward();
	
	// to screen
	vec2f World2Screen(const vec3f& world);
	
	// to viewport
	vec2f Screen2Viewport(const vec2f& screen);

	// to world at z0
	vec3f Viewport2WorldAtZ0(const vec2f& viewport);
	vec3f Screen2WorldAtZ0(const vec2f& screen);

	// to ray
	Rig3D::Ray<vec3f> Viewport2Ray(const vec2f& viewport);
	Rig3D::Ray<vec3f> Screen2Ray(const vec2f& screen);

private:
	//For Screen2World/Pixel2Unit calculation
	Rig3D::Ray<vec3f> pRay;
	Rig3D::Plane<vec3f> pPlane;
	float pDist;
};

