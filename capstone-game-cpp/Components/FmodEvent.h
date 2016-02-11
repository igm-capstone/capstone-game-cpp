#pragma once
#include "BaseComponent.h"
#include "fmodwrap.h"

class FmodEvent : public BaseComponent
{
	friend class Factory<FmodEvent>;

public:
	FMOD::Studio::EventInstance* mInstance;

	void Load(std::string path);
	void Unload();
	void Play();

private:
	FMOD::Studio::System* mSystem;

	FmodEvent();
	~FmodEvent();
};

