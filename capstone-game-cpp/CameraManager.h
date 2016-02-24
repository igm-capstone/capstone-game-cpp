#pragma once
#include "Uniforms.h"
#include <Rig3D/Graphics/Camera.h>
#include <Rig3D/Parametric.h>


class CameraManager
{
	Rig3D::Renderer* mRenderer;
	Rig3D::Camera mCameraPersp;
	Rig3D::Camera mCameraQuadOrto;
	Rig3D::Camera mCameraScreenOrto;

	CBuffer::Camera	mCBufferPersp;
	CBuffer::Camera	mCBufferScreenOrto;
	CBuffer::Camera	mCBufferFullLevelOrto;
	vec3f mForward;
	vec3f mOrigin;
	vec3f mLookAt;

	vec2f mLevelCenter;
	vec2f mLevelExtends;


	const float mFOV = 0.33f * PI;
	const float mNearPlane = 0.1f;
	const float mFarPlane = 200;

protected:
	CameraManager();
	~CameraManager();

public:

	vec3f GetOrigin() { return mOrigin; };
	vec3f GetLookAt() { return mLookAt; };

	CBuffer::Camera* GetCBufferPersp() { return mIsOrto ? &mCBufferFullLevelOrto : &mCBufferPersp; }
	CBuffer::Camera* GetCBufferScreenOrto() { return mIsOrto ? &mCBufferFullLevelOrto : &mCBufferScreenOrto; }
	CBuffer::Camera* GetCBufferFullLevelOrto() { return &mCBufferFullLevelOrto; }

	bool mIsOrto;

	void SetLevelBounds(vec2f center, vec2f extends);
	void ChangeLookAtTo(const vec3f& newLookAt);
	void ChangeLookAtBy(const vec3f& offset);
	void MoveCamera(const vec3f& lookAt, const vec3f& origin);
	void OnResize();
	
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

