#pragma once
#pragma once
#include "Behavior.h"

namespace BehaviorTree
{

	class Composite : public Behavior
	{
	public:
		Composite(Tree& tree, std::string name);

		void Dump(std::stringstream& ss, int level = 0)
		{
			ss << std::string(level, '\t') << mName << " [" << mStatus << "]" << std::endl;
		}

		std::vector<Behavior*> mChildren;

		void Add(Behavior& behavior);
	};

}
