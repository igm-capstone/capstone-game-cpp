#include "IEventHandler.h"

class Eventt : public IEvent
{
public:

	uint32_t ID;
	const char* Message;
	Eventt(uint32_t id, const char* message) : ID(id), Message(message) {};
	Eventt(){};
	~Eventt(){};
};

class A
{
public:
	A(){};
	~A(){};
};


class Obs : public A, public virtual IObserver
{
public:

	Obs(){};
	~Obs(){};

	void HandleEvent(const IEvent& iEvent) override
	{
		printf("Received event: %u at %p\n", static_cast<const Eventt&>(iEvent).ID, &*this);
	}
};

class EV : public IEventHandler
{
public:

	void NotifyObservers(uint32_t eventID) override
	{
		ObserverMap::iterator mapEntry = mObservers.find(eventID);
		if (mapEntry == mObservers.end())
		{
			return;
		}

		for (ObserverSet::const_iterator iter = mapEntry->second.begin(); iter != mapEntry->second.end(); iter++)
		{
			Eventt e = Eventt(eventID, "Event");
			(*iter)->HandleEvent(e);
		}
	}
};

int main()
{
	Obs o1 = Obs();
	Obs o2 = Obs();
	Obs o3 = Obs();

	EV e = EV();

	uint32_t eventIDs[2] = { 0, 1 };

	e.RegisterObserver(eventIDs[0], &o1);
	e.RegisterObserver(eventIDs[1], &o2);
	e.RegisterObserver(eventIDs[0], &o3);
	e.NotifyObservers(eventIDs[0]);
	e.NotifyObservers(eventIDs[1]);

	getchar();
}