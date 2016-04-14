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

	mSpriteManager->DrawSprite(SPRITESHEET_PANELS, numBtns - 1, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(512, 128));

	for (auto i = 0; i < numBtns; i++)
	{
		RenderButton(&mButtons[i], mSpriteManager->perc2f(0.5f, posYPerc) + vec2f(-64.0f * (numBtns-1) + 128.0f * i, 0));
	}
}

void SkillBar::RenderButton(Button* b, vec2f pos)
{
	mSpriteManager->DrawSprite(b->sheetID, 8+b->spriteID, pos, vec2f(100, 100));
	mSpriteManager->DrawSprite(b->sheetID, b->spriteID, pos, vec2f(100, 100), vec3f(1,1), 2*PI * b->skill->Recharged());
	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 16, pos + vec2f(0,30), vec3f(1, 1, 1), ALIGN_CENTER, "%s", b->skill->mName);
	if (b->skill->mCost) mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 25, pos + vec2f(-25, -43), vec3f(0, 0.61f, 0.88f), ALIGN_CENTER, "%.0f", b->skill->mCost);
	if (b->keySpriteID != -1) mSpriteManager->DrawSprite(SPRITESHEET_CONTROL_ICONS, b->keySpriteID, pos + vec2f(30, -33), vec2f(28, 28));
	if (b->isHighlighted) mSpriteManager->DrawSprite(b->sheetID, 15, pos, vec2f(100, 100));
	
}

void SkillBar::AddSkill(Skill* skill, SpriteSheetCode sheetID, int spriteID, int keySpriteID)
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
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 2, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(450, 56), vec2f(ghost.GetManaPerc(), 1));
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 0, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(450, 56));
	}
}
