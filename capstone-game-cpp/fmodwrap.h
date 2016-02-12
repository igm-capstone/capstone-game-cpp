#pragma once

#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include "trace.h"

#pragma comment (lib, "fmodL_vc.lib")
#pragma comment (lib, "fmodstudioL_vc.lib")

static FMOD_RESULT __sResult;

#define FMOD_CHECK(result) \
if ((__sResult = (result)) != FMOD_RESULT::FMOD_OK) \
{ \
	TRACE_ERROR("FMOD error " << __sResult << " - " << FMOD_ErrorString(__sResult) << " (at: " << __FILE__ << "," << __LINE__ << ")" ); \
}


#define FMOD_CHECK_RESULT(result) \
if ((__sResult = (result)) != FMOD_RESULT::FMOD_OK) \
{ \
	TRACE_ERROR("FMOD error " << __sResult << " - " << FMOD_ErrorString(__sResult) << "! (at: " << __FILE__ << "," << __LINE__ << ")" ); \
    return int(__sResult); \
}
