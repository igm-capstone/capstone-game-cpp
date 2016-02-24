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

class Behavior;
typedef void(*ObserverCallback)(Behavior&, void*, BehaviorStatus);
typedef BehaviorStatus(*UpdateCallback)(Behavior&, void*);
typedef void(*FlushCallback)(Behavior&, void*);
typedef void(*InitializeCallback)(Behavior&, void*);
typedef void(*TerminateCallback)(Behavior&, void*, BehaviorStatus);

struct BehaviorObserver
{
	ObserverCallback callback;
	Behavior*        behavior;
	void*            data;

	static BehaviorObserver Default() { return{ nullptr, nullptr, nullptr }; }
};

class Behavior
{
	friend class BehaviorTree;

public:

	Behavior();
	~Behavior();

	void SetInitializeCallback(InitializeCallback callback)
	{
		mOnInitialize = callback;
	}

	void SetFlushCallback(FlushCallback callback)
	{
		mOnFlush = callback;
	}

	void SetUpdateCallback(UpdateCallback callback)
	{
		mOnUpdate = callback;
	}

	void SetTerminateCallback(TerminateCallback callback)
	{
		mOnTerminate = callback;
	}

	BehaviorStatus GetStatus() const
	{
		return mStatus;
	}

	void SetObserver(BehaviorObserver observer)
	{
		mObserver = observer;
	}

	void NotifyObserver(BehaviorStatus status) const
	{
		if (mObserver.callback)
		{
			mObserver.callback(*mObserver.behavior, mObserver.data, status);
		}
	}

	BehaviorStatus Tick(void* userData);

protected:
	BehaviorStatus     mStatus;
	BehaviorObserver   mObserver;

	UpdateCallback     mOnUpdate;
	FlushCallback      mOnFlush;
	InitializeCallback mOnInitialize;
	TerminateCallback  mOnTerminate;
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
		SetInitializeCallback(&OnInitialize);
		SetUpdateCallback(&OnUpdate);
		SetTerminateCallback(&OnTerminate);
	}

	~MockBehavior()
	{
	}

	static void OnInitialize(Behavior& bh, void* data)
	{
		auto& self = static_cast<MockBehavior&>(bh);

		++self.mInitializeCalled;
	}

	static void OnTerminate(Behavior& bh, void* data, BehaviorStatus status)
	{
		auto& self = static_cast<MockBehavior&>(bh);

		++self.mTerminateCalled;
		self.mTerminateStatus = status;
	}

	static BehaviorStatus OnUpdate(Behavior& bh, void* data)
	{
		auto& self = static_cast<MockBehavior&>(bh);

		++self.mUpdateCalled;
		return self.mReturnStatus;
	}
};
