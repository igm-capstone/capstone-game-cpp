#include "stdafx.h"
#include "FmodEvent.h"
#include <ScareTacticsApplication.h>

using namespace FMOD::Studio;
void FmodEvent::Load(std::string path)
{
	EventDescription* loopingAmbienceDescription;
	FMOD_CHECK(mStudio->getEvent(path.c_str(), &loopingAmbienceDescription));
	//loopingAmbienceDescription->loadSampleData();

	FMOD_CHECK(loopingAmbienceDescription->createInstance(&mInstance));
}

void FmodEvent::Unload()
{
	// Release will clean up the instance when it completes
	FMOD_CHECK(mInstance->release());
}

FmodEvent::FmodEvent() : mInstance(nullptr)
{
	mStudio = Application::SharedInstance().mStudio;
}


FmodEvent::~FmodEvent()
{
}
