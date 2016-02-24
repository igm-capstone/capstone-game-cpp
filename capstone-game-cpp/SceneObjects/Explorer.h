#pragma once
#include "SceneObjects\BaseSceneObject.h"

void InitializeExplorerInventory();

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	class NetworkID*					mNetworkID;
	class ExplorerController*			mController;
	class AnimationController*			mAnimationController;
	class SphereColliderComponent*		mCollider;
	class BaseColliderComponent*		mMeleeCollider;	// Not sure where this will be
	class Skill**						mSkills;
	class Health*						mHealth;

private:
	class NetworkClient*				mNetworkClient;
	class CameraManager*				mCameraManager;

private:
	Explorer();
	~Explorer() {};

public:
	void Spawn(vec3f pos, int UUID);

	static void OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);
	static void OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnNetHealthChange(BaseSceneObject* obj, float newVal);
	static void OnHealthChange(BaseSceneObject* obj, float newVal);
	static void OnCollisionExit(BaseSceneObject* obj, BaseSceneObject* other);

	static void DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static void DoMelee(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static void OnMeleeStart(void* obj);
	static void OnMeleeStop(void* obj);

};
