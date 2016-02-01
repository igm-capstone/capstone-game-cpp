#pragma once
#include <GraphicsMath/cgm.h>

struct CbufferPVM
{
	struct CameraData
	{
		mat4f projection;
		mat4f view;
	} camera;

	mat4f world;
};