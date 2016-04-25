#include "SceneObjects/BaseSceneObject.h"


class Door : public BaseSceneObject
{
	friend class Factory<Door>;

public:
	class OrientedBoxColliderComponent* mColliderComponent;
	class OrientedBoxColliderComponent* mTrigger;
	class NetworkID*      mNetworkID;
	class NetworkManager* mNetworkManager;

	void SetID(uint16_t id);

private:

	Door();
	~Door();

	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other);
	static void OnInteract(BaseSceneObject* obj);
public:
	void Interact();

	void ToggleDoor();
};
