#include "SceneObjects/BaseSceneObject.h"
#include "Components/ColliderComponent.h"
#include "Explorer.h"
#include "Components/ExplorerController.h"

enum LampStatus
{
	LAMP_OFF,
	LAMP_ON,
	LAMP_DIMMED
};
class Lamp : public BaseSceneObject
{
	friend class Factory<Lamp>;

public:
	IMesh*						mConeMesh;
	vec4f mLightColor;
	vec3f mLightDirection;
	float mLightRadius;							// Linear range of spot light
	float mLightAngle;

	OrientedBoxColliderComponent* mTrigger;
	LampStatus mStatus;

private:
	Lamp() :
		mConeMesh(nullptr),
		mLightRadius(0.0f),
		mLightAngle(0.0f),
		mTrigger(Factory<OrientedBoxColliderComponent>::Create()),
		mStatus(LAMP_OFF)
	{
		mTrigger->mIsTrigger = true;
		mTrigger->mIsDynamic = false;
		mTrigger->mIsActive = true;
		mTrigger->mSceneObject = this;
		mTrigger->RegisterTriggerStayCallback(OnTriggerStay);
	}

	~Lamp()
	{
	}

	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other)
	{
		Lamp* lamp = static_cast<Lamp*>(obj);
		if (other->Is<Explorer>())
		{
			Explorer* e = static_cast<Explorer*>(other);
			if (e->mController->mIsInteracting) {
				e->mController->ConsumeInteractWill();
				lamp->ToggleLamp();
			}
		}
	}

public:
	void ToggleLamp() {
		switch (mStatus)
		{
		case LAMP_OFF:
			mStatus = LAMP_ON;
			break;
		case LAMP_ON:
			mStatus = LAMP_DIMMED;
			break;
		case LAMP_DIMMED:
			mStatus = LAMP_ON;
			break;
		}
	}
};