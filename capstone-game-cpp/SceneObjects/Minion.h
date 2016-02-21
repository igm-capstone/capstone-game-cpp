#pragma once
#include "BaseSceneObject.h"

class Minion : public BaseSceneObject
{
	friend class Factory<Minion>;

public:
	class NetworkID*				mNetworkID;
	class MinionController*			mController;
	class SphereColliderComponent*	mCollider;
	class AnimationController*		mAnimationController;

private:
	Minion();
	~Minion();

public:
	void Spawn(vec3f pos, int UUID);;

	static void OnMove(BaseSceneObject* obj, vec3f newPos);
	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);
};

