#include "stdafx.h"
#include "GhostController.h"
#include <SceneObjects/Minion.h>
#include <ScareTacticsApplication.h>
#include <SceneObjects/Ghost.h>


GhostController::GhostController()
{
}


GhostController::~GhostController()
{
}


void GhostController::DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	auto ghost = reinterpret_cast<Ghost*>(obj);
	ghost->mEvent->Play();

	NetworkCmd::SpawnNewMinion(pos);
}
