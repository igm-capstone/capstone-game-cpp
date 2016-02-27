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

private:

	Door() : mCanOpen(false), mColliderComponent(Factory<OrientedBoxColliderComponent>::Create())
	{
		mColliderComponent->mIsTrigger = false;
		mColliderComponent->mIsDynamic = false;
		mColliderComponent->mIsActive = true;
		mColliderComponent->mSceneObject = this;

		mColliderComponent->RegisterCollisionExitCallback(OnCollisionExit);
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
				door->mColliderComponent->mIsActive = !door->mColliderComponent->mIsActive;
		}
	}
};
