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

	~DominationPoint()
	{
		Factory<SphereColliderComponent>::Destroy(mCollider);
		Factory<DominationPointController>::Destroy(mController);
	}

	static void OnTriggerEnter(BaseSceneObject* obj, BaseSceneObject* other)
	{
		auto dom = static_cast<DominationPoint*>(obj);
		if (dom->mController->isDominated || !other->Is<Explorer>())
		{
			return;
		}

		auto exp = static_cast<Explorer*>(other);
		dom->mController->AddExplorer(exp);
	}

	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
	{

	}

	static void OnTriggerExit(BaseSceneObject* obj, BaseSceneObject* other)
	{
		auto dom = static_cast<DominationPoint*>(obj);
		if (dom->mController->isDominated || !other->Is<Explorer>())
		{
			return;
		}
		
		auto exp = static_cast<Explorer*>(other);
		dom->mController->RemoveExplorer(exp);
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
