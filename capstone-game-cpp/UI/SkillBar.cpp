#include "stdafx.h"
#include "SkillBar.h"
#include "SpriteManager.h"
#include <SceneObjects/Ghost.h>


SkillBar::SkillBar()
{
	mSpriteManager = &Rig3D::Singleton<SpriteManager>::SharedInstance();
}


SkillBar::~SkillBar()
{
}

void SkillBar::RenderPanel()
{
	auto posYPerc = 0.07f;

	auto startPt = 0.5f - 0.04f * (numBtns - 1);

	mSpriteManager->DrawSpriteAtPerc(3, numBtns - 1, vec2f(0.5f, posYPerc), vec2f(512, 128));

	for (auto i = 0; i < numBtns; i++)
	{
		RenderButton(&mButtons[i], vec2f(startPt, posYPerc));
		startPt += 0.04f * 2.0f;
	}
}

void SkillBar::RenderButton(Button* b, vec2f pos)
{
	mSpriteManager->DrawSpriteAtPerc(b->sheetID, 8+b->spriteID, pos, vec2f(100, 100));
	mSpriteManager->DrawSpriteAtPerc(b->sheetID, b->spriteID, pos, vec2f(100, 100), vec3f(1,1), 2*PI * b->skill->Recharged());
	if (b->keySpriteID != -1) mSpriteManager->DrawSpriteAtPerc(4, b->keySpriteID, pos + vec2f(0.02f, 0.035f), vec2f(28, 28));
	if (b->isHighlighted) mSpriteManager->DrawSpriteAtPerc(b->sheetID, 15, pos, vec2f(100, 100));
	
}

void SkillBar::AddSkill(Skill* skill, int sheetID, int spriteID, int keySpriteID)
{
	assert(numBtns < 4);

	mButtons[numBtns].sheetID = sheetID;
	mButtons[numBtns].spriteID = spriteID;
	mButtons[numBtns].skill = skill;
	mButtons[numBtns].keySpriteID = keySpriteID;

	numBtns++;
}

void SkillBar::SetActive(Skill* skill)
{
	for (auto i = 0; i < numBtns; i++)
	{
		mButtons[i].isHighlighted = (mButtons[i].skill == skill);
	}
}

void SkillBar::RenderManaBar()
{
	auto posYPerc = 0.95f;
	for each (Ghost& ghost in Factory<Ghost>()) {
		mSpriteManager->DrawSpriteAtPerc(0, 2, vec2f(0.5f, posYPerc), vec2f(450, 56), vec2f(ghost.GetManaPerc(), 1));
		mSpriteManager->DrawSpriteAtPerc(0, 0, vec2f(0.5f, posYPerc), vec2f(450, 56));
	}
}
