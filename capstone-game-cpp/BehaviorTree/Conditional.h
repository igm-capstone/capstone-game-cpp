#pragma once
#include "Decorator.h"

namespace BehaviorTree
{

	class Conditional : public Decorator
	{
	public:
		Conditional(Tree& tree, Behavior* child, class Predicate* predicate, std::string name = "Conditional");

		void SetPredicate(Predicate* predicate)
		{
			mPredicate = predicate;
		}

	protected:
		class Predicate* mPredicate;

		static void OnInitialize(Behavior& bh, void* data);
		static BehaviorStatus OnUpdate(Behavior& bh, void* data);
		static void OnChildComplete(Behavior& bh, void* data, BehaviorStatus status);
	};

}
