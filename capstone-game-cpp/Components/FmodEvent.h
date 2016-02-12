#pragma once
#include "BaseComponent.h"
#include "fmodwrap.h"


enum class FmodEventType
{
	SINGLE_INSTANCE = 1,
	FIRE_AND_FORGET = 2,
};


class FmodEvent : public BaseComponent
{
	friend class Factory<FmodEvent>;
	friend class FmodEventCollection;

public:

	void Load(std::string path, FmodEventType eventType);
	void Unload();
	
	void Play();
	void Pause();
	void Resume();
	bool IsPaused();
	void Stop(bool immediate = false);

	FMOD::Studio::EventInstance* GetInstance();

private:
	FMOD::Studio::EventDescription* mDescription;
	FMOD::Studio::EventInstance* mInstance;
	FMOD::Studio::System* mStudio;
	FmodEventType mType;

	FmodEvent();
	~FmodEvent();
};

