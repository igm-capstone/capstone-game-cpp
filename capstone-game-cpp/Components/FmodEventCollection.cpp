#include "stdafx.h"
#include "FmodEventCollection.h"


using namespace std;

void FmodEventCollection::Register(string id, string path, FmodEventType type)
{
	auto fmodEvent = Factory<FmodEvent>::Create();
	fmodEvent->mSceneObject = mSceneObject;
	fmodEvent->Load(path, type);

	mEventMap[id] = fmodEvent;
}

void FmodEventCollection::UnloadAll()
{
	for (auto fmodEvent : mEventMap)
	{
		fmodEvent.second->Unload();
	}
}

void FmodEventCollection::Play(string id)
{
	mEventMap[id]->Play();
}

void FmodEventCollection::Stop(string id)
{
	mEventMap[id]->Stop();
}

void FmodEventCollection::PauseAll()
{
	for (auto fmodEvent : mEventMap)
	{
		fmodEvent.second->Pause();
	}
}

void FmodEventCollection::ResumeAll()
{
	for (auto fmodEvent : mEventMap)
	{
		fmodEvent.second->Resume();
	}
}

void FmodEventCollection::StopAll()
{
	for (auto fmodEvent : mEventMap)
	{
		fmodEvent.second->Stop();
	}
}

FmodEvent* FmodEventCollection::operator[](string id)
{
	return mEventMap[id];
}

FmodEventCollection::FmodEventCollection() : 
	mEventMap(unordered_map<string, FmodEvent*>())
{
}

FmodEventCollection::~FmodEventCollection()
{
	for (auto fmodEvent : mEventMap)
	{
		fmodEvent.second->~FmodEvent();
	}
}
