#include "stdafx.h"
#include "NetworkManager.h"
#include <Factory.h>
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Explorer.h>
#include <SceneObjects/Minion.h>
#include <Components/NetworkID.h>
#include <SceneObjects/Heal.h>
#include <SceneObjects/Trap.h>
#include <SceneObjects/StatusEffect.h>
#include <ScareTacticsApplication.h>
#include <Mathf.h>
#include <SceneObjects/Lantern.h>

NetworkManager::NetworkManager()
{
	
}

NetworkManager::~NetworkManager()
{
}

bool NetworkManager::StartServer()
{
	if (mMode == IDLE) {
		if (mServer.Init()) {
			mMode = SERVER;
			return true;
		}
	}
	return false;
}

bool NetworkManager::StopServer()
{
	if (mMode == SERVER) {
		mServer.Shutdown();
		mMode = IDLE;
		return true;
	}
	return false;
}

bool NetworkManager::StartClient()
{
	if (mMode == IDLE) {
		if (mClient.Init()) {
			mMode = CLIENT;
			return true;
		}
	}
	return false;
}

bool NetworkManager::StopClient()
{
	if (mMode == CLIENT) {
		mClient.Shutdown();
		mMode = IDLE;
		return true;
	}
	return false;
}

void NetworkManager::Update()
{
	if (mMode == SERVER) {
		mServer.Update();
	} 
	else if (mMode == CLIENT) {
		mClient.Update();
	}
}

int MyUUID::currentID = 10;
NetworkManager* NetworkCmd::mNetworkManager = &Rig3D::Singleton<NetworkManager>::SharedInstance();
NetworkManager* NetworkRpc::mNetworkManager = &Rig3D::Singleton<NetworkManager>::SharedInstance();

void NetworkCmd::SpawnNewExplorer(int clientID) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER);
	// Get a spawn point
	// FIMXE: logic to select spawn point
	SpawnPoint& sp = *(Factory<SpawnPoint>().begin());
	auto rndPos = sp.mTransform->GetPosition() + vec3f(Mathf::RandomRange(-4, 4), Mathf::RandomRange(-4, 4), 0);

	auto e = Factory<Explorer>::Create();
	e->Spawn(rndPos, clientID);

	Packet p(PacketTypes::SPAWN_EXPLORER);
	p.UUID = e->mNetworkID->mUUID;
	p.AsTransform.Position = rndPos;
	mNetworkManager->mServer.SendToAll(&p);

	Packet p2(PacketTypes::GRANT_AUTHORITY);
	p2.UUID = e->mNetworkID->mUUID;
	mNetworkManager->mServer.Send(clientID, &p2);

	//Send existing elements
	for each(Explorer &exp in Factory<Explorer>()) {
		if (&exp == e) continue;
		p.UUID = exp.mNetworkID->mUUID;
		p.AsTransform.Position = exp.mTransform->GetPosition();
		mNetworkManager->mServer.Send(clientID, &p);
	}

	auto ui = &Application::SharedInstance().GetCurrentScene()->mUIManager;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		Packet p3(READY);
		p3.ClientID = i;
		p3.AsBool = ui->GetReadyState(i);
		mNetworkManager->mServer.Send(clientID, &p3);
	}
}

void NetworkRpc::DisconnectExplorer(int UUID)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == UUID) {
			Explorer* e = static_cast<Explorer*>(netID.mSceneObject);
			Factory<Explorer>::Destroy(e);
		}
	}
}

void NetworkRpc::SpawnExistingExplorer(int UUID, vec3f pos) {
	assert(mNetworkManager->mMode == NetworkManager::Mode::CLIENT);
	
	Explorer* e = nullptr;
	// Prevent duplicates
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == UUID) {
			e = static_cast<Explorer*>(netID.mSceneObject);
		}
	}

	if (!e) e = Factory<Explorer>::Create();

	e->Spawn(pos, UUID);
}

void NetworkCmd::SpawnNewSkill(SkillPacketTypes type, vec3f pos, float duration)
{
	assert(mNetworkManager->mMode == NetworkManager::Mode::SERVER);

	int UUID = MyUUID::GenUUID();

	switch (type)
	{
	case SKILL_TYPE_HEAL:
	{
		auto h = Factory<Heal>::Create();
		if (h)
		{
			h->mNetworkID->mHasAuthority = true;
			h->Spawn(pos, UUID, duration);
			h->mNetworkID->OnNetAuthorityChange(true);
		}
		break;
	}
	case SKILL_TYPE_POISON:
	{
		auto p = Factory<Trap>::Create();
		if (p)
		{
			p->mNetworkID->mHasAuthority = true;
			p->SpawnPoison(UUID, pos, duration);
			p->mNetworkID->OnNetAuthorityChange(true);
		}
		break;
	}
	case SKILL_TYPE_SLOW:
	{
		auto s = Factory<Trap>::Create();
		if (s)
		{
			s->mNetworkID->mHasAuthority = true;
			s->SpawnSlow(UUID, pos, duration);
			s->mNetworkID->OnNetAuthorityChange(true);
		}
		break;
	}
	case SKILL_TYPE_LANTERN:
	{
		auto l = Factory<Lantern>::Create();
		if (l)
		{
			l->mNetworkID->mHasAuthority = true;
			l->Spawn(UUID, pos, duration);
			l->mNetworkID->OnNetAuthorityChange(true);
		}
		break;
	}
	case SKILL_TYPE_FLYTRAP_MINION:
	{
		auto f = Factory<Minion>::Create();
		if (f)
		{
			f->mNetworkID->mHasAuthority = true;
			f->SpawnFlytrap(pos, UUID);
			f->mNetworkID->OnNetAuthorityChange(true);
		}
		break;
	}
	case SKILL_TYPE_IMP_MINION:
	case SKILL_TYPE_ABOMINATION_MINION: // for now
	{
		auto m = Factory<Minion>::Create();
		if (m)
		{
			m->mNetworkID->mHasAuthority = true;
			m->SpawnImp(pos, UUID);
			m->mNetworkID->OnNetAuthorityChange(true);	
		}
		break;
	}
	case SKILL_TYPE_UNKNOWN:
	default:
		break;
	}
	
	Packet p(PacketTypes::SPAWN_SKILL);
	p.UUID = UUID;
	p.AsSkill.Position = pos;
	p.AsSkill.Duration = duration;
	p.AsSkill.Type = type;
	mNetworkManager->mServer.SendToAll(&p);
}

void NetworkRpc::SpawnExistingSkill(SkillPacketTypes type, int UUID, vec3f pos, float duration)
{
	assert(mNetworkManager->mMode == NetworkManager::Mode::CLIENT);

	switch (type)
	{
	case SKILL_TYPE_HEAL:
	{
		auto h = Factory<Heal>::Create();
		if (h)
		{
			h->Spawn(pos, UUID, duration);
		}
		break;
	}
	case SKILL_TYPE_POISON:
	{
		auto p = Factory<Trap>::Create();
		if (p)
		{
			p->SpawnPoison(UUID, pos, duration);
		}
		break;
	}
	case SKILL_TYPE_SLOW:
	{
		auto s = Factory<Trap>::Create();
		if (s)
		{
			s->SpawnSlow(UUID, pos, duration);
		}
		break;
	}
	case SKILL_TYPE_LANTERN:
	{
		auto l = Factory<Lantern>::Create();
		if (l)
		{
			l->Spawn(UUID, pos, duration);
		}
		break;
	}
	case SKILL_TYPE_FLYTRAP_MINION:
	{
		auto f = Factory<Minion>::Create();
		if (f)
		{
			f->SpawnFlytrap(pos, UUID);
		}
		break;
	}
	case SKILL_TYPE_IMP_MINION:
	case SKILL_TYPE_ABOMINATION_MINION:
	{
		auto m = Factory<Minion>::Create();
		if (m)
		{
			m->SpawnImp(pos, UUID);
		}
		break;
	}
	case SKILL_TYPE_UNKNOWN:
	default:
		break;
	}
}

void NetworkRpc::GrantAuthority(int UUID) {
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mIsActive && netID.mUUID == UUID) {
			netID.mHasAuthority = true;
			netID.OnNetAuthorityChange(true);
		}
	}
}

void NetworkRpc::SyncTransform(int UUID, vec3f pos, quatf rot)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnNetSyncTransform(pos, rot);
		}
	}
}

void NetworkRpc::SyncHealth(int UUID, float val, float max, float dir)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnNetHealthChange(val, max, dir);
		}
	}
}

void NetworkRpc::SyncAnimation(int UUID, byte state, byte command)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnNetSyncAnimation(state, command);
		}
	}
}

void NetworkRpc::Interact(int UUID)
{
	for each(auto &netID in Factory<NetworkID>()) {
		if (netID.mUUID == UUID) {
			netID.OnInteract();
		}
	}
}

void NetworkRpc::Ready(unsigned clientID, bool isReady)
{
	Application::SharedInstance().GetCurrentScene()->mUIManager.SetReadyState(clientID, isReady);
}
