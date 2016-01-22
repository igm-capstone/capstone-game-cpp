#pragma once
#include "Scenes/BaseScene.h"

class Level01 : public BaseScene
{
public:
	void VOnResize() override;
	
	void VInitialize() override;
	void InitializeShaderResources();
	
	void VUpdate(double milliseconds) override;
	
	void VRender() override;
	
	void VShutdown() override;
};