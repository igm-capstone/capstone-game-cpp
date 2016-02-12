#pragma once
#include "Uniforms.h"
#include <Rig3D/Graphics/Camera.h>
#include <Rig3D/Parametric.h>


class CameraManager
{
	Rig3D::Renderer* mRenderer;
	Rig3D::Camera mCameraOrto;
	Rig3D::Camera mCameraPersp;

	CBufferCamera	mCBufferOrto;
	CBufferCamera	mCBufferPersp;
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

	CBufferCamera* GetCBufferPersp() { return &mCBufferPersp; }
	CBufferCamera* GetCBufferOrto() { return &mCBufferOrto; }


	void ChangeLookAtTo(const vec3f& newLookAt);
	void ChangeLookAtBy(const vec3f& offset);
	void MoveCamera(const vec3f& lookAt, const vec3f& origin);
	void OnResize();
	vec3f GetForward();
	vec2f World2Screen(vec3f world);
	vec3f Screen2WorldAt0(vec2f screen);
	Rig3D::Ray<vec3f> Screen2Ray(vec2f screen);
	float PixelToWorldSize(float pixel);

	float pPixel2Unit;
private:
	//For Screen2World/Pixel2Unit calculation
	Rig3D::Ray<vec3f> pRay;
	Rig3D::Plane<vec3f> pPlane;
	float pDist;
};

