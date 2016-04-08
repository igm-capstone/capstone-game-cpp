#include "stdafx.h"
#include "SkillBar.h"
#include "SpriteManager.h"


SkillBar::SkillBar()
{
	mSpriteManager = &Rig3D::Singleton<SpriteManager>::SharedInstance();
}


SkillBar::~SkillBar()
{
}

void SkillBar::RenderPanel()
{
	mSpriteManager->DrawSpriteAtPerc(3, 1, vec2f(0.5f, 0.05f), vec2f(0.5f, 0.1f));
}
