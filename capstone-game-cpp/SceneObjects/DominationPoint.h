#pragma once
#include "BaseSceneObject.h"


class DominationPoint : public BaseSceneObject
{
	friend class Factory<DominationPoint>;
public:

	float mDominationTime;
	int mTier;

private:
	DominationPoint() : mDominationTime(0.0f), mTier(0) {}
	~DominationPoint() {}
};