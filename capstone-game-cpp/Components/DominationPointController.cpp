#include "stdafx.h"
#include "DominationPointController.h"
#include <SceneObjects/DominationPoint.h>
#include <SceneObjects/Explorer.h>
#include "Mathf.h"
#include <Colors.h>

void DominationPointController::Update(double milliseconds)
{
	if (isDominated)
	{
		return;
	}

	int count = 0;
	for (auto exp : mExplorers)
	{
		if (exp == nullptr || exp->mIsDead)
		{
			continue;
		}

		// Success only if no predicate was assigned or the assigned predicate is true.
		if (!mActivationPredicate || mActivationPredicate(exp))
		{
			count++;
		}
	}

	auto dom = static_cast<DominationPoint*>(mSceneObject);
	auto dt = static_cast<float>(milliseconds) * 0.001f;

	auto rad = dom->mCollider->mCollider.radius;
	auto pos = dom->mTransform->GetPosition();

	//TRACE_LINE(pos + vec3f(0, rad, 0), pos + vec3f(0, -rad, 0), Colors::red);
	//TRACE_LINE(pos + vec3f(rad, 0, 0), pos + vec3f(-rad, 0, 0), Colors::red);

	auto prog = rad * mProgress * 0.72f;
	//TRACE_LINE(pos + vec3f( prog,  prog, 0), pos + vec3f(-prog, -prog, 0), Colors::green);
	//TRACE_LINE(pos + vec3f( prog, -prog, 0), pos + vec3f(-prog,  prog, 0), Colors::green);


	// progress variation
	float dp = 0;
	if (count > 0)
	{
		switch (mActivationType)
		{
		case ActivationType::Single:
			dp += dt * mBaseProgressRate;
			break;
		case ActivationType::Multiple:
			dp += dt * (mBaseProgressRate + mBonusProgressRate * (count - 1));
			break;
		}
	}
	else
	{
		dp -= dt * mDecayRate;
	}

	float newProgress = clamp01(mProgress + dp);

	if (mProgress == newProgress)
	{
		return;
	}

	// TODO: Fire ChangeProgress callback
		
	mProgress = newProgress;
	
	if (mProgress == 1)
	{
		// TODO: Fire Dominated callback
		isDominated = true;
	}
}

bool DominationPointController::AddExplorer(Explorer* newExplorer)
{
	Explorer** slot = nullptr;
	for (auto& exp : mExplorers)
	{
		// dont add if already in the list
		if (exp == newExplorer)
		{
			return false;
		}
		else if (slot == nullptr && exp == nullptr)
		{
			slot = &exp;
		}

	}

	if (slot)
	{
		*slot = newExplorer;
		return true;
	}

	return false;
}

bool DominationPointController::RemoveExplorer(Explorer* newExplorer)
{
	for (auto& exp : mExplorers)
	{
		if (exp == newExplorer)
		{
			exp = nullptr;
			return true;
		}
	}

	return false;
}

DominationPointController::DominationPointController() :
	mBaseProgressRate(.2f),
	mBonusProgressRate(0),
	mDecayRate(.05f),
	mActivationType(ActivationType::Single),
	mActivationPredicate(nullptr),
	mProgress(0),
	isDominated(false)
{
	memset(mExplorers, 0, sizeof(mExplorers));
}


DominationPointController::~DominationPointController()
{
}
