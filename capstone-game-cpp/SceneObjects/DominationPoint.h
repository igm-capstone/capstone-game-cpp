#pragma once
#include "BaseSceneObject.h"
#include "Explorer.h"
#include "CollisionManager.h"
#include <Components/DominationPointController.h>
#include <Components/ExplorerController.h>


class DominationPoint : public BaseSceneObject
{
	friend class Factory<DominationPoint>;

public:


	SphereColliderComponent* mCollider;
	DominationPointController* mController;

	// legacy data
	float mDominationTime;
	int mTier;

private:
	DominationPoint() : 
		mDominationTime(0.0f), 
		mTier(0)
	{
		mCollider = Factory<SphereColliderComponent>::Create();
		mCollider->mSceneObject = this;
		mCollider->mCollider.radius = 5;
		mCollider->mIsActive = true;
		mCollider->mIsTrigger = true;
		mCollider->RegisterTriggerEnterCallback(&OnTriggerEnter);
		mCollider->RegisterTriggerStayCallback(&OnTriggerStay);
		mCollider->RegisterTriggerExitCallback(&OnTriggerExit);

		mController = Factory<DominationPointController>::Create();
		mController->mSceneObject = this;
		mController->mActivationPredicate = &IsExplorerInteracting;
	}

	~DominationPoint() {}

	// TODO: change this to collision stay as soon as it is implemented
	static void OnTriggerEnter(BaseSceneObject* obj, BaseSceneObject* other)
	{
		TRACE_LOG("OnTriggerEnter");

	}

	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
	{
		TRACE_LOG("OnTriggerStay");

		auto dom = static_cast<DominationPoint*>(obj);
		if (dom->mController->isDominated)
		{
			return;
		}

		// I cant know out of the box if a class is 
		// subtype of other if we dont have polymorphic types.
		// For this case, I'm assuming that we only have collisions
		// between DominationPoints AND EXPLORER.
		// This code needs to be revised otherwise.
		auto exp = static_cast<Explorer*>(other);

		dom->mController->AddExplorer(exp);
	}

	static void OnTriggerExit(BaseSceneObject* obj, BaseSceneObject* other)
	{
		TRACE_LOG("OnTriggerExit");

	}

	static bool IsExplorerInteracting(Explorer* exp)
	{
		if (exp == nullptr || exp->mController == nullptr)
		{
			return false;
		}

		//TRACE_LOG("Interact " << exp->mController->mIsInteracting);
		return exp->mController->mIsInteracting;
	}
};
