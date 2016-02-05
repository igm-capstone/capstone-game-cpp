#include "stdafx.h"
#include "GhostController.h"
#include <SceneObjects/Minion.h>
#include <ScareTacticsApplication.h>


GhostController::GhostController()
{
}


GhostController::~GhostController()
{
}

void GhostController::DoSpawnMinion(BaseSceneObject* obj, float duration, BaseSceneObject* target, vec3f pos)
{
	NetworkCmd::SpawnNewMinion(pos);
}
