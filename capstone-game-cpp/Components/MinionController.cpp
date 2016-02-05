#include "stdafx.h"
#include "MinionController.h"


MinionController::MinionController()
{
}


MinionController::~MinionController()
{
}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;

	bool hasMoved = false;

	float speed = mSpeed;

	auto pos = mSceneObject->mTransform->GetPosition();
	// move in the explorer direction

	if (hasMoved)
		OnMove(pos);

	return hasMoved;
}