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

private:

	Door() : mCanOpen(false), mBoxCollider(Factory<OrientedBoxColliderComponent>::Create())
	{
		mBoxCollider->mIsTrigger = false;
		mBoxCollider->mIsDynamic = false;
		mBoxCollider->mIsActive = true;
		mBoxCollider->mSceneObject = this;

		mBoxCollider->RegisterCollisionExitCallback(OnCollisionExit);
	}

	~Door()
	{
	}

	static void OnCollisionExit(BaseSceneObject* obj, BaseSceneObject* other)
	{
		Door* door = static_cast<Door*>(obj);
		if (other->Is<Explorer>())
		{
			Explorer* e = static_cast<Explorer*>(other);
			if (e->mController->mIsInteracting && door->mCanOpen)
				door->mBoxCollider->mIsActive = !door->mBoxCollider->mIsActive;
		}
	}
};
