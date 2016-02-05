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
	CameraData	camera;
	vec4f		color;
	float		cosAngle;
	float		range;
	float		padding[2];
};