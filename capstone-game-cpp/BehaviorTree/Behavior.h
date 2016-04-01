#pragma once
#include <string>
#include <sstream>
#include <functional>

namespace BehaviorTree
{
	enum class BehaviorStatus
	{
		Invalid,
		Success,
		Failure,
		Running,
	};
}

inline std::ostream& operator<<(std::ostream& out, const BehaviorTree::BehaviorStatus& value) {
	static std::map<BehaviorTree::BehaviorStatus, std::string> strings;
	if (strings.size() == 0) {
		
		using namespace BehaviorTree;
		int idx = -1;
		std::string str;

#define INSERT_ELEMENT(p)               \
		str = std::string(#p);          \
		idx = str.find_last_of(':');    \
		strings[p] = str.substr(idx+1);

		INSERT_ELEMENT(BehaviorStatus::Invalid)
		INSERT_ELEMENT(BehaviorStatus::Success)
		INSERT_ELEMENT(BehaviorStatus::Failure)
		INSERT_ELEMENT(BehaviorStatus::Running)

#undef INSERT_ELEMENT
	}

	return out << strings[value];
}

namespace BehaviorTree
{
	class Behavior;
	typedef void(*ObserverCallback)(Behavior&, void*, BehaviorStatus);
	typedef BehaviorStatus(*UpdateCallback)(Behavior&, void*);
	typedef void(*ResetCallback)(Behavior&, void*);
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
		friend class Tree;

	public:

		Behavior(Tree& tree, std::string name = "Behavior");
		~Behavior();

		void Dump(std::stringstream& ss, int level = 0)
		{
			mDumpCallback(ss, level);
		}

		void SetInitializeCallback(InitializeCallback callback)
		{
			mOnInitialize = callback;
		}

		void SetResetCallback(ResetCallback callback)
		{
			mOnReset = callback;
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

		BehaviorStatus Tick(void* userData = nullptr);
		void Reset(void* userData);

	protected:
		Tree&              mTree;
		std::string        mName;
		BehaviorStatus     mStatus;
		BehaviorObserver   mObserver;

		UpdateCallback     mOnUpdate;
		ResetCallback      mOnReset;
		InitializeCallback mOnInitialize;
		TerminateCallback  mOnTerminate;

		std::function<void(std::stringstream&, int)> mDumpCallback;
	};
}