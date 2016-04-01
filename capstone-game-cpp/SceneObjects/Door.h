#include "SceneObjects/BaseSceneObject.h"
#include "Explorer.h"
#include "Components/ExplorerController.h"
#include <trace.h>

class Door : public BaseSceneObject
{
	friend class Factory<Door>;

public:
	bool mCanOpen;
	OrientedBoxColliderComponent* mColliderComponent;
	BoxColliderComponent* mTrigger;

private:

	Door() : mCanOpen(false), mColliderComponent(Factory<OrientedBoxColliderComponent>::Create()), mTrigger(Factory<BoxColliderComponent>::Create())
	{
		mColliderComponent->mIsTrigger = false;
		mColliderComponent->mIsDynamic = false;
		mColliderComponent->mIsActive = true;
		mColliderComponent->mSceneObject = this;

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
				door->ToogleDoor();
			}
		}
	}

public:
	void ToogleDoor()
	{
		if (!mCanOpen) return;
		mColliderComponent->mIsActive ? mTransform->RotateRoll(0.5f*PI) : mTransform->RotateRoll(-0.5f*PI);
		mColliderComponent->mIsActive = !mColliderComponent->mIsActive;
	}
};
