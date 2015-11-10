#pragma once
#include <functional>
#include <unordered_map>
#include <set>
#include <stdint.h>

#pragma warning (disable: 4251)

#ifdef _WINDLL
#define IEVENT_API __declspec(dllexport)
#else
#define IEVENT_API __declspec(dllimport)
#endif

class IEVENT_API IEvent
{
public:
	IEvent() {};
	virtual ~IEvent() = 0 {};
};

class IEVENT_API IObserver
{
public:
	IObserver() {};
	virtual ~IObserver() {};

	virtual void HandleEvent(const IEvent& iEvent) = 0;
};

typedef std::set<IObserver*>						ObserverSet;
typedef std::unordered_map<uint32_t, ObserverSet>	ObserverMap;

class IEVENT_API IEventHandler
{
public:
	IEventHandler();
	virtual ~IEventHandler();

	void RegisterObserver(uint32_t eventID, IObserver* observer);
	void UnregisterObserver(uint32_t eventID, IObserver* observer);
	virtual void NotifyObservers(const IEvent& iEvent) = 0;

protected:
	ObserverMap mObservers;
};

namespace std
{
	template<>
	struct IEVENT_API hash<IObserver>
	{
		size_t operator()(const IObserver& observer) const
		{
			return reinterpret_cast<size_t>(&observer);
		}
	};

	template<>
	struct IEVENT_API equal_to<IObserver>
	{
		bool operator()(const IObserver& lhs, const IObserver& rhs) const
		{
			return (&lhs == &rhs);
		}
	};
}