#include "Camera.h"

using namespace Rig3D;

Camera::Camera()
{
}

Camera::~Camera()
{
}

mat4f Camera::GetProjectionMatrix()
{
	return mProjection;
}

mat4f Camera::GetViewMatrix()
{
	return mView;
}

void Camera::SetProjectionMatrix(const mat4f& projection)
{
	mProjection = projection;
}

void Camera::SetViewMatrix(const mat4f& view)
{
	mView = view;
	mIsDirty = true;
}

vec3f Camera::GetForward()
{
	if (mIsDirty)
	{
		mForward = vec3f(0, 0, 1) * mat3f(mView.inverse());
	}

	return mForward;
}
