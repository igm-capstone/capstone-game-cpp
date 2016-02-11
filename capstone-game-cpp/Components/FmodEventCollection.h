#pragma once
#include "BaseComponent.h"
#include "FmodEvent.h"

class FmodEventCollection : public BaseComponent
{
	friend class Factory<FmodEventCollection>;

public:
	void Register(std::string id, std::string path, FmodEventType type);
	void UnloadAll();

	void Play(std::string id);
	void Stop(std::string id);

	void PauseAll();
	void ResumeAll();
	void StopAll();

	FmodEvent* operator[](std::string id);

private:
	std::unordered_map<std::string, FmodEvent*> mEventMap;

	FmodEventCollection();
	~FmodEventCollection();
};

