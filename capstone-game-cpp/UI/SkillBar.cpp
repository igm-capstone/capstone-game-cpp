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
	auto posYPerc = 0.07f;

	auto startPt = 0.5f - 0.04f * (numBtns - 1);

	mSpriteManager->DrawSpriteAtPerc(3, numBtns - 1, vec2f(0.5f, posYPerc), vec2f(0.5f, 0.5f));

	for each (Button &b in mButtons)
	{
		RenderButton(&b, vec2f(startPt, posYPerc));
		startPt += 0.04f * 2.0f;
	}
}

void SkillBar::RenderButton(Button* b, vec2f pos)
{
	mSpriteManager->DrawSpriteAtPerc(b->sheetID, b->spriteID, pos, vec2f(0.4f, 0.4f));
	if (b->isHighlighted) mSpriteManager->DrawSpriteAtPerc(b->sheetID, 15, pos, vec2f(0.4f, 0.4f));
}

void SkillBar::AddSkill(Skill* skill, int sheetID, int spriteID)
{
	assert(numBtns < 4);

	mButtons[numBtns].sheetID = sheetID;
	mButtons[numBtns].spriteID = spriteID;
	mButtons[numBtns].skill = skill;
	
	numBtns++;
}

void SkillBar::SetActive(Skill* skill)
{
	for each (Button &b in mButtons)
	{
		b.isHighlighted = (b.skill == skill);
	}
}
