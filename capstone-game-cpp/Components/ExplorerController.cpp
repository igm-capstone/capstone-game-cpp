#include <stdafx.h>
#include "ExplorerController.h"
#include <SceneObjects/Explorer.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Intersection.h>

bool ExplorerController::Move()
{
	if (!mIsActive) return false;

	Explorer* explorer = static_cast<Explorer*>(mSceneObject);
	auto input = (&Singleton<Engine>::SharedInstance())->GetInput();
	auto networkManager = &Singleton<NetworkManager>::SharedInstance();

	bool hasMoved = false;

	auto pos = explorer->mTransform->GetPosition();
	if (input->GetKey(KEYCODE_LEFT))
	{
		pos.x -= mSpeed;
		hasMoved = true;
	}
	if (input->GetKey(KEYCODE_RIGHT))
	{
		pos.x += mSpeed;
		hasMoved = true;
	}
	if (input->GetKey(KEYCODE_UP))
	{
		pos.y += mSpeed;
		hasMoved = true;
	}
	if (input->GetKey(KEYCODE_DOWN))
	{
		pos.y -= mSpeed;
		hasMoved = true;
	}

	/*BoxCollider aabb = { pos, explorer->mCollider->mCollider.halfSize };
	bool canMove = true;
	for (int i = 0; i < mWallCount; i++)
	{
		if (IntersectAABBAABB(aabb, mWallColliders[i].mCollider))
		{
			canMove = false;
			break;
		}
	}*/

	if (hasMoved) {
		explorer->mTransform->SetPosition(pos);
		explorer->mCollider->mCollider.origin = pos;

		if (explorer->mNetworkID->mHasAuthority) {
			Packet p(PacketTypes::SYNC_TRANSFORM);
			p.UUID = explorer->mNetworkID->mUUID;
			p.Position = pos;
			networkManager->mClient.SendData(&p);
		}
	}

	return hasMoved;
}
