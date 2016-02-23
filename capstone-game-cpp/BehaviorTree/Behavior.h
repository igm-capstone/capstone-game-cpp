#pragma once

#include <functional>
#include "TestSuite.h"

enum class BehaviorStatus
{
	Invalid,
	Success,
	Failure,
	Running,
	Suspended,
};

namespace std
{
	inline ostream& operator<<(std::ostream& out, const BehaviorStatus& value) {
		static std::map<BehaviorStatus, std::string> strings;
		if (strings.size() == 0) {
#define INSERT_ELEMENT(p) strings[p] = #p
		INSERT_ELEMENT(BehaviorStatus::Invalid);
		INSERT_ELEMENT(BehaviorStatus::Success);
		INSERT_ELEMENT(BehaviorStatus::Failure);
		INSERT_ELEMENT(BehaviorStatus::Running);
		INSERT_ELEMENT(BehaviorStatus::Suspended);
#undef INSERT_ELEMENT
		}

		return out << strings[value];
	}
}


typedef std::function<void(BehaviorStatus)> BehaviorObserver;

class Behavior
{
public:
	typedef BehaviorStatus(*UpdateCallback)();
	typedef void(*InitializeCallback)();
	typedef void(*TerminateCallback)(BehaviorStatus status);

	Behavior();
	virtual ~Behavior();

	BehaviorStatus Tick();
	virtual BehaviorStatus Update() = 0;
	virtual void OnInitialize() {}
	virtual void OnTerminate(BehaviorStatus) {}


protected:
	BehaviorStatus     mStatus;
	BehaviorObserver   mObserver;
	UpdateCallback     mUpdateCallback;
	InitializeCallback mInitializeCallback;
	TerminateCallback  mTerminateCallback;
};


struct MockBehavior : public Behavior
{
	int mInitializeCalled;
	int mTerminateCalled;
	int mUpdateCalled;
	BehaviorStatus mReturnStatus;
	BehaviorStatus mTerminateStatus;

	MockBehavior()
		: mInitializeCalled(0)
		, mTerminateCalled(0)
		, mUpdateCalled(0)
		, mReturnStatus(BehaviorStatus::Running)
		, mTerminateStatus(BehaviorStatus::Invalid)
	{
	}

	virtual ~MockBehavior()
	{
	}

	virtual void OnInitialize() override
	{
		++mInitializeCalled;
	}

	virtual void OnTerminate(BehaviorStatus s) override
	{
		++mTerminateCalled;
		mTerminateStatus = s;
	}

	virtual BehaviorStatus Update() override
	{
		++mUpdateCalled;
		return mReturnStatus;
	}
};
