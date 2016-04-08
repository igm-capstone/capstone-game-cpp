#pragma once
#include "BaseSceneObject.h"

class Minion : public BaseSceneObject
{
	friend class Factory<Minion>;

public:
	class NetworkClient*			mNetworkClient;
	class NetworkID*				mNetworkID;
	class MinionController*			mController;
	class SphereColliderComponent*	mCollider;
	class SphereColliderComponent*  mMeleeColliderComponent;
	class AnimationController*		mAnimationController;

private:
	Minion();
	~Minion();

public:
	void Spawn(vec3f pos, int UUID);

	void UpdateComponents(quatf rotation, vec3f position);

	static void OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);

};

