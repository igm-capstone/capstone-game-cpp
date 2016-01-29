#pragma once
#include "Scenes/BaseScene.h"
#include "capstone-game-cpp/CollisionManager.h"

class Level01 : public BaseScene
{
	mat4f* mWalls;
	short mWallCount;
	ID3D11Buffer*					mWallInstanceBuffer;
	
	CollisionManager	mCollisionManager;

public:
	void VOnResize() override;
	
	void VInitialize() override;
	void InitializeShaderResources();
	
	void VUpdate(double milliseconds) override;
	
	void VRender() override;
	
	void VShutdown() override;
};