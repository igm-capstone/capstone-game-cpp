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

private:
	Ghost();
	~Ghost() {};

public:
	void Spawn(vec3f pos, int UUID);

	static void DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos);
};
