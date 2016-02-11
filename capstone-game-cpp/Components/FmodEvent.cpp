#include "stdafx.h"
#include "FmodEvent.h"
#include <ScareTacticsApplication.h>

using namespace FMOD::Studio;
void FmodEvent::Load(std::string path, FmodEventType type)
{
	Unload();
	mType = type;

	FMOD_CHECK(mStudio->getEvent(("event:/" + path).c_str(), &mDescription));

	// single instance events have their instance loaded right away
	// while fire and forget events preload the data and create a
	// new instance at each play.
	switch (mType)
	{
	case FmodEventType::SINGLE_INSTANCE:
		FMOD_CHECK(mDescription->createInstance(&mInstance));
		break;
	case FmodEventType::FIRE_AND_FORGET:
		FMOD_CHECK(mDescription->loadSampleData());
		break;
	default:
		break;
	}
}

void FmodEvent::Unload()
{
	// Release will clean up the instance when it completes
	if (mInstance && mType == FmodEventType::SINGLE_INSTANCE)
	{
		FMOD_CHECK(mInstance->release());
	}

	mDescription = nullptr;
	mType = FmodEventType(0);
}

void FmodEvent::Play()
{
	switch (mType)
	{
	case FmodEventType::SINGLE_INSTANCE:
		FMOD_CHECK(mInstance->start());
		break;
	case FmodEventType::FIRE_AND_FORGET:
		FMOD_CHECK(mDescription->createInstance(&mInstance));
		FMOD_CHECK(mInstance->start());
		FMOD_CHECK(mInstance->release());
		break;
	default:
		break;
	}
}

void FmodEvent::Pause()
{
	if (mInstance)
	{
		FMOD_CHECK(mInstance->setPaused(true));
	}
}

void FmodEvent::Resume()
{
	if (mInstance)
	{
		FMOD_CHECK(mInstance->setPaused(false));
	}
}

bool FmodEvent::IsPaused()
{
	auto isPaused = false;
	
	if (mInstance)
	{
		FMOD_CHECK(mInstance->getPaused(&isPaused));
	}

	return isPaused;
}

void FmodEvent::Stop(bool immediate)
{
	// NOTE: I dont know what happens if stop is called on an event that was already been destroyed
	if (mInstance)
	{
		FMOD_CHECK(mInstance->stop(immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT));
	}
}

EventInstance* FmodEvent::GetInstance()
{
	return mInstance;
}

FmodEvent::FmodEvent() : 
	mDescription(nullptr), 
	mInstance(nullptr), 
	mType(FmodEventType(0))
{
	mStudio = Application::SharedInstance().mStudio;
}


FmodEvent::~FmodEvent()
{
	Unload();
}
