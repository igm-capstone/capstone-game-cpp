#include "SceneObjects/BaseSceneObject.h"
#include "Explorer.h"
#include "Components/ExplorerController.h"
#include <trace.h>

class Door : public BaseSceneObject
{
	friend class Factory<Door>;

public:
	OrientedBoxColliderComponent* mColliderComponent;
	OrientedBoxColliderComponent* mTrigger;

private:

	Door() : mColliderComponent(Factory<OrientedBoxColliderComponent>::Create()), mTrigger(Factory<OrientedBoxColliderComponent>::Create())
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
		Factory<OrientedBoxColliderComponent>::Destroy(mColliderComponent);
		Factory<OrientedBoxColliderComponent>::Destroy(mTrigger);
	}

	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
	{
		Door* door = static_cast<Door*>(obj);
		if (other->Is<Explorer>())
		{
			Explorer* e = static_cast<Explorer*>(other);
			if (e->mController->mIsInteracting) {
				e->mController->ConsumeInteractWill();
				door->ToogleDoor();
			}
		}
	}

public:
	void ToogleDoor()
	{
		mColliderComponent->mIsActive ? mTransform->RotateRoll(0.5f*PI) : mTransform->RotateRoll(-0.5f*PI);
		mColliderComponent->mIsActive = !mColliderComponent->mIsActive;
	}
};
