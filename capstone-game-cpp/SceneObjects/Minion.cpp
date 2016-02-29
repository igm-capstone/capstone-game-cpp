#include "stdafx.h"
#include "Minion.h"
#include <Components/NetworkID.h>
#include <Components/ColliderComponent.h>
#include <Components/MinionController.h>
#include <Components/AnimationController.h>
#include <ScareTacticsApplication.h>
#include <Vertex.h>
#include <Components/AnimationUtility.h>
#include <Network\NetworkClient.h>

Minion::Minion()
{
	mNetworkClient = &Singleton<NetworkManager>::SharedInstance().mClient;

	mNetworkID = Factory<NetworkID>::Create();
	mNetworkID->mSceneObject = this;
	mNetworkID->mIsActive = false;
	mNetworkID->RegisterNetAuthorityChangeCallback(&OnNetAuthorityChange);

	mCollider = Factory<SphereColliderComponent>::Create();
	mCollider->mSceneObject = this;
	mCollider->mIsActive = false;
	mCollider->mIsDynamic = true;

	mController = Factory<MinionController>::Create();
	mController->mSceneObject = this;
	mController->mIsActive = false;
	mController->RegisterMoveCallback(&OnMove);
	
	Application::SharedInstance().GetModelManager()->GetModel(kMinionAnimModelName)->Link(this);

	mAnimationController = Factory<AnimationController>::Create();
	mAnimationController->mSceneObject = this;
	mAnimationController->mSkeletalAnimations = &mModel->mSkeletalAnimations;
	mAnimationController->mSkeletalHierarchy = mModel->mSkeletalHierarchy;
	SetRestFrameIndex(mAnimationController, gMinionRestFrameIndex);
}

Minion::~Minion()
{
}

void Minion::Spawn(vec3f pos, int UUID)
{
	mTransform->SetPosition(pos);

	mCollider->mIsActive = true;
	mCollider->mCollider.origin = pos;

	mNetworkID->mIsActive = true;
	mNetworkID->mUUID = UUID;
}

void Minion::OnMove(BaseSceneObject* obj, vec3f newPos)
{
	auto m = static_cast<Minion*>(obj);
	m->mTransform->SetPosition(newPos);
	m->mCollider->mCollider.origin = newPos;


	if (m->mNetworkID->mHasAuthority) {
		Packet p(PacketTypes::SYNC_TRANSFORM);
		p.UUID = m->mNetworkID->mUUID;
		p.AsTransform.Position = newPos;
		m->mNetworkClient->SendData(&p);

		//m->mHealth->TakeDamage(1.0f);
		//if (m->mHealth->GetHealth() <= 0) e->mHealth->TakeDamage(-1000.0f);
	}
}

void Minion::OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth)
{
	auto m = static_cast<Minion*>(obj);
	m->mController->mIsActive = newAuth;
}
