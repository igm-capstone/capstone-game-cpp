#pragma once
#include "Uniforms.h"


class CameraManager
{
	CBufferCamera	mCameraOrto;
	CBufferCamera	mCameraPersp;

	vec3f mOrigin;
	vec3f mLookAt;
	float mAspectRatio;
	vec3f mForward;

public:
	CameraManager();
	~CameraManager();

	vec3f GetOrigin() { return mOrigin; };
	vec3f GetLookAt() { return mLookAt; };

	CBufferCamera* GetPerspCameraData() { return &mCameraPersp; }
	CBufferCamera* GetOrtoCameraData() { return &mCameraOrto; }


	void Initialize(const float aspectRatio);
	void ChangeLookAtTo(const vec3f& newLookAt);
	void ChangeLookAtBy(const vec3f& offset);
	void MoveCamera(const vec3f& lookAt, const vec3f& origin);
	void ChangeAspectRatio(const float aspectRatio);
	vec3f GetForward();
};

