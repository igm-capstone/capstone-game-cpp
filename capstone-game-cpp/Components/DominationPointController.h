#pragma once
#include "BaseComponent.h"

class DominationPointController : public BaseComponent
{
	friend class Factory<DominationPointController>;

public:
	enum class ActivationType {
		Single = 1,
		Multiple = 2,
	};

	typedef bool (*ActivationPredicate)(class Explorer*);

	// progress rate per second.
	float mBaseProgressRate;

	// bonus progress rate per second added 
	// to the base progress rate for each 
	// extra explorer.
	float mBonusProgressRate;

	// Decay Rate per second when no explorer
	// is present.
	float mDecayRate;

	// Single or multiple explorers.
	ActivationType mActivationType;

	// Condition to progress.
	ActivationPredicate mActivationPredicate;

	class Explorer* mExplorers[MAX_EXPLORERS];
	float mProgress;
	bool isDominated;

	void Update(float milliseconds);
	bool AddExplorer(Explorer* exp);
	bool RemoveExplorer(Explorer* exp);

private:

	DominationPointController();
	~DominationPointController();
};

