#pragma once
#include "Behavior.h"
#include "Predicate.h"
#include <sstream>

namespace
{
	struct MockBehavior : public BehaviorTree::Behavior
	{
		int mInitializeCalled;
		int mTerminateCalled;
		int mUpdateCalled;
		int mResetCalled;
		BehaviorTree::BehaviorStatus mReturnStatus;
		BehaviorTree::BehaviorStatus mTerminateStatus;

		MockBehavior(BehaviorTree::Tree& tree, std::string name = "Mock Behavior")
			: Behavior(tree, name)
			, mInitializeCalled(0)
			, mTerminateCalled(0)
			, mUpdateCalled(0)
			, mResetCalled(0)
			, mReturnStatus(BehaviorTree::BehaviorStatus::Running)
			, mTerminateStatus(BehaviorTree::BehaviorStatus::Invalid)
		{
			SetInitializeCallback(&OnInitialize);
			SetUpdateCallback(&OnUpdate);
			SetTerminateCallback(&OnTerminate);
			SetResetCallback(&OnReset);
		}

		~MockBehavior()
		{
		}

		static void OnInitialize(Behavior& bh, void* data)
		{
			auto& self = static_cast<MockBehavior&>(bh);

			++self.mInitializeCalled;
		}

		static void OnTerminate(Behavior& bh, void* data, BehaviorTree::BehaviorStatus status)
		{
			auto& self = static_cast<MockBehavior&>(bh);

			++self.mTerminateCalled;
			self.mTerminateStatus = status;
		}

		static BehaviorTree::BehaviorStatus OnUpdate(Behavior& bh, void* data)
		{
			auto& self = static_cast<MockBehavior&>(bh);

			++self.mUpdateCalled;
			return self.mReturnStatus;
		}

		static void OnReset(Behavior& bh, void* data)
		{
			auto& self = static_cast<MockBehavior&>(bh);

			++self.mResetCalled;
		}


	};


	template <class COMPOSITE>
	class MockComposite : public COMPOSITE
	{
	public:
		MockComposite(class BehaviorTree::Tree& tree, size_t size, std::string name = "Mock Composite")
			: COMPOSITE(tree, name)
		{
			for (size_t i = 0; i < size; ++i)
			{
				std::stringstream childName;
				childName << "Mock Behavior [" << i << "]";
				COMPOSITE::Add(*new MockBehavior(tree, childName.str()));
			}
		}

		~MockComposite()
		{
			for (size_t i = 0; i < COMPOSITE::mChildren.size(); ++i)
			{
				delete COMPOSITE::mChildren[i];
			}
		}

		MockBehavior& operator[](size_t index)
		{
			assert(index < COMPOSITE::mChildren.size());
			return *static_cast<MockBehavior*>(COMPOSITE::mChildren[index]);
		}
	};


	struct MockPredicate : public BehaviorTree::Predicate
	{
		bool mPredicateResult;

		MockPredicate(class BehaviorTree::Tree& tree, std::string name = "Mock Predicate")
			: Predicate(tree, name)
			, mPredicateResult(true)
		{
			SetPredicateCallback(OnPredicate);
		}

		static bool OnPredicate(Behavior& bh, void* data)
		{
			auto& self = reinterpret_cast<MockPredicate&>(bh);
			return self.mPredicateResult;
		}
	};
}
