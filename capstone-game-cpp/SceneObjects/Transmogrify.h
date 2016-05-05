#pragma once
#include "SceneObjects\BaseSceneObject.h"

class Transmogrify : public BaseSceneObject
{
	friend class Factory<Transmogrify>;

public:
	class AnimationController* mAnimationController;
	class AnimationController* mTargetAnimationController;

	class ModelCluster* mTargetModelCluster;

	float mDuration;

	void Spawn(float duration, int targetUUID);
	void Update(float seconds);
	void Attach(class Explorer* pExplorer);
	void Detach();

private:
	Transmogrify();
	~Transmogrify();
};

