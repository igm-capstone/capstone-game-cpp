#pragma once
#include "Behavior.h"

namespace BehaviorTree
{

	typedef bool(*PredicateCallback)(Behavior& self, void* data);

	class Predicate : public Behavior
	{
	public:
		Predicate(Tree& tree, std::string name = "Predicate");

		void SetPredicateCallback(PredicateCallback callback)
		{
			mOnPredicate = callback;
		}

		bool IsValid() const
		{
			return !mOnPredicate || mStatus != BehaviorStatus::Failure;
		}

		static BehaviorStatus OnUpdate(Behavior& self, void* data);

	protected:
		PredicateCallback mOnPredicate;
	};

}

