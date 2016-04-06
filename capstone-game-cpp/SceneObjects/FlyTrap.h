#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include <Components/AnimationController.h>

class FlyTrap :
	public BaseSceneObject
{
	friend class Factory<FlyTrap>;
public:

	class NetworkClient*			mNetworkClient;
	class NetworkID*				mNetworkID;
	class FlyTrapController*		mController;
	class SphereColliderComponent*	mCollider;
	class AnimationController*		mAnimationController;

private:
	FlyTrap();
	~FlyTrap();

public:
	void Spawn(vec3f pos, int UUID);;

	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);
	static void OnAnimationCommandExecuted(BaseSceneObject* obj, AnimationControllerState state, AnimationControllerCommand command);
	static void OnNetSyncAnimation(BaseSceneObject* obj, byte state, byte command);

	static void OnMeleeStart(void* obj);
	static void OnMeleeStop(void* obj);

	static void OnMeleeHit(BaseSceneObject* self, BaseSceneObject* other);
};

