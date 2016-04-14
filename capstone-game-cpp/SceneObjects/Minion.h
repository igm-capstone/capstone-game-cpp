#pragma once
#include "BaseSceneObject.h"

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
	class NetworkClient*			mNetworkClient;
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
	static void OnHealthChange(BaseSceneObject* obj, float newVal, bool shouldCheckAuthority);
	static void OnNetHealthChange(BaseSceneObject* obj, float newVal);
};

