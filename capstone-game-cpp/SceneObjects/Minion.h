#pragma once
#include "BaseSceneObject.h"
#include <Components/AnimationController.h>

class Minion : public BaseSceneObject
{
	friend class Factory<Minion>;

public:
	enum MinionClass
	{
		UNKNOWN,
		IMP,
		ABOMINATION,
		FLYTRAP,
	};

	MinionClass						mClass;
	class NetworkServer*			mNetworkServer;
	class NetworkID*				mNetworkID;
	class MinionController*			mController;
	class SphereColliderComponent*	mCollider;
	class SphereColliderComponent*  mMeleeColliderComponent;
	class AnimationController*		mAnimationController;
	class Health*					mHealth;

private:
	Minion();
	~Minion();

	void Spawn(vec3f pos, int UUID);
public:
	
	void SpawnImp(vec3f pos, int UUID);
	void SpawnAbomination(vec3f pos, int UUID);
	void SpawnFlytrap(vec3f pos, int UUID);

	MinionClass GetClass() const { return mClass; }

	void UpdateComponents(quatf rotation, vec3f position);

	static void OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);
	static void OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnHealthChange(BaseSceneObject* obj, float newVal);
	static void OnDeath(BaseSceneObject* obj);
	//static void OnNetHealthChange(BaseSceneObject* obj, float newVal);
	static void OnAnimationCommandExecuted(BaseSceneObject* obj, AnimationControllerState state, AnimationControllerCommand command);
	static void OnNetSyncAnimation(BaseSceneObject* obj, byte state, byte command);

};

