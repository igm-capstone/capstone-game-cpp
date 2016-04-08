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
	class SkillBar*				mSkillBar;
	int							mActiveSkill;

private:
	Ghost();
	~Ghost() {};

public:
	void Spawn(class BaseScene* scene);
	void SetActiveSkill(int skillNum);

	static void DoMouseClick(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	
	static void DoSpawnBasicMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static void DoSpawnBomberMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static void DoSpawnPlantMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
	static void DoTransmogrify(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);

private:
	static void DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos, int minionType);
};
