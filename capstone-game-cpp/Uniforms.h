#pragma once
#include <GraphicsMath/cgm.h>

struct CameraData
{
	mat4f projection;
	mat4f view;
};

struct CbufferPVM
{
	CameraData camera;
	mat4f world;
};

struct CBufferLight
{
	mat4f		viewProjection;
	vec4f		color;
	vec3f		direction;
	float		cosAngle;
	float		range;
	float		padding[3];
};