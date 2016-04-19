#include "SceneObjects/BaseSceneObject.h"

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
	class OrientedBoxColliderComponent* mTrigger;
	class IMesh*          mConeMesh;
	class NetworkID*      mNetworkID;
	class NetworkManager* mNetworkManager;

	vec4f mLightColor;
	vec3f mLightDirection;
	float mLightRadius;							// Linear range of spot light
	float mLightAngle;
	LampStatus mStatus;

private:
	Lamp();
	~Lamp();
	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other);
	static void OnInteract(BaseSceneObject* obj);
public:
	void SetID(uint16_t id);
	void Interact();
	void ToggleLamp();
};