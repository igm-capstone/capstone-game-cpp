#include "SceneObjects/BaseSceneObject.h"
#include "Explorer.h"
#include "Components/ExplorerController.h"
#include <trace.h>

class Door : public BaseSceneObject
{
	friend class Factory<Door>;

public:
	bool mCanOpen;
	OrientedBoxColliderComponent* mBoxCollider;
	BoxColliderComponent* mTrigger;

private:

	Door() : mCanOpen(false), mBoxCollider(Factory<OrientedBoxColliderComponent>::Create()), mTrigger(Factory<BoxColliderComponent>::Create())
	{
		mBoxCollider->mIsTrigger = false;
		mBoxCollider->mIsDynamic = false;
		mBoxCollider->mIsActive = true;
		mBoxCollider->mSceneObject = this;

		mTrigger->mIsTrigger = true;
		mTrigger->mIsDynamic = false;
		mTrigger->mIsActive = true;
		mTrigger->mSceneObject = this;

		mTrigger->RegisterTriggerStayCallback(OnTriggerStay);
	}

	~Door()
	{
	}

	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
	{
		Door* door = static_cast<Door*>(obj);
		if (other->Is<Explorer>())
		{
			Explorer* e = static_cast<Explorer*>(other);
			if (e->mController->mIsInteracting && door->mCanOpen) {
				e->mController->ConsumeInteractWill();
				door->mBoxCollider->mIsActive ? door->mTransform->RotateRoll(0.5f*PI) : door->mTransform->RotateRoll(-0.5f*PI);
				door->mBoxCollider->mIsActive = !door->mBoxCollider->mIsActive;
			}
		}
	}
};
