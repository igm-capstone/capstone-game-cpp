#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include <Components/AnimationController.h>

enum ExplorerType : uint8_t // For Spawn Order, check gSpawnOrder in NetworkServer.cpp
{
	GHOST = 0, // padding
	HEALER,
	SPRINTER,
	TRAPMASTER,
};

class Explorer : public BaseSceneObject
{
	friend class Factory<Explorer>;
public:
	union MeleeColliderComponent
	{
		class BaseColliderComponent*		asBaseColliderComponent;
		class SphereColliderComponent*		asSphereColliderComponent;
		class OrientedBoxColliderComponent* asBoxColliderComponent;
	};

	class NetworkID*					mNetworkID;
	class ExplorerController*			mController;
	class AnimationController*			mAnimationController;
	class SphereColliderComponent*		mCollider;
	class SphereColliderComponent*		mInteractionCollider;
	class Skill*						mSkills[MAX_EXPLORER_SKILLS];
	class Health*						mHealth;
	class Node*							mCurrentNode;

	MeleeColliderComponent				mMeleeColliderComponent;
	float								mAttackDamage;
	ExplorerType						mExplorerType;
	bool								mIsDead;
	bool								mIsTransmogrified;

private:
	class NetworkClient*				mNetworkClient;
	class CameraManager*				mCameraManager;

private:
	Explorer();
	~Explorer();

public:
	ExplorerType GetExplorerType() { return mExplorerType; }

	void Spawn(vec3f pos, int UUID);
	void DebugSpawn(vec3f pos, int UUID);
	void UpdateComponents(quatf rotation, vec3f position);

	static void OnMove(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);
	static void OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	//static void OnNetHealthChange(BaseSceneObject* obj, float newVal);
	static void OnAnimationCommandExecuted(BaseSceneObject* obj, AnimationControllerState state, AnimationControllerCommand command);
	static void OnNetSyncAnimation(BaseSceneObject* obj, byte state, byte command);
	static void OnHealthChange(BaseSceneObject* obj, float oldVal, float newVal, float hitDirection);
	static void OnDeath(BaseSceneObject* obj);
	static void OnRevive(BaseSceneObject* obj);
	static void OnCollisionExit(BaseSceneObject* obj, BaseSceneObject* other);
	static void OnTriggerStay(BaseSceneObject* obj, BaseSceneObject* other);

	static bool DoSprint(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static bool DoMelee(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static bool DoHeal(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static bool DoPoison(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static bool DoSlow(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);
	static bool DoLantern(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f worldPosition);

	static void OnMeleeStart(void* obj);
	static void OnMeleeStop(void* obj);

	static void OnMeleeHit(BaseSceneObject* self, BaseSceneObject* other);

};
