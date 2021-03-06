#pragma once
#include "SceneObjects\BaseSceneObject.h"

class Ghost : public BaseSceneObject
{
	friend class Factory<Ghost>;
	friend class GhostController;

public:
	class NetworkID*			mNetworkID;
	class GhostController*		mController;
	class Skill*				mSkills[MAX_GHOST_SKILLS];
	class FmodEventCollection*	mEvents;

private:
	class CameraManager*		mCameraManager;
	class UIManager*			mUIManager;
	int							mActiveSkill;
	float						mMana;
	float						mMaxMana;
	float						mManaRegen[4];
	float						mManaRegenFrequency;
	int							mManaRegenLevel;

private:
	Ghost();
	~Ghost();

public:
	void Spawn(class BaseScene* scene);
	int GetActiveSkill();
	void SetActiveSkill(int skillNum);
	void TickMana(float milliseconds);
	float GetMana() { return mMana; };
	float GetManaPerc() { return mMana/mMaxMana; };

	static bool DoMouseClick(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static bool DoSpawnImpMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static bool DoSpawnAbominationMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static bool DoSpawnFlytrapMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static bool DoTransmogrify(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);

private:
	static bool DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos, SkillPacketTypes minionType);
};
