#include "stdafx.h"
#include "SkillBar.h"
#include "SpriteManager.h"
#include <Scenes/Level01.h>
#include <SceneObjects/DominationPoint.h>
#include <SceneObjects/Ghost.h>
#include <Components/NetworkID.h>
#include <Components/GhostController.h>


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
	mSpriteManager->DrawSprite(b->sheetID, b->spriteID, pos, vec2f(100, 100), vec4f(1,1,1,1), vec3f(1,1), 2*PI * b->skill->Recharged());
	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 16, pos + vec2f(0,30), vec4f(1, 1, 1, 1), ALIGN_CENTER, "%s", b->skill->mName);
	if (b->skill->mCost) mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 25, pos + vec2f(-25, -43), vec4f(0, 0.61f, 0.88f, 1), ALIGN_CENTER, "%.0f", b->skill->mCost);
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
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 2, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(450, 56), vec4f(1, 1, 1, 1), vec2f(ghost.GetManaPerc(), 1));
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 0, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(450, 56));
	}
}

void SkillBar::RenderObjectives(GameState gameState, bool isServer)
{
	auto anchor = mSpriteManager->perc2f(0, 0.85f) + vec2f(10, 0);
	int i = 0;

	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 14, anchor + vec2f(10, 0), vec4f(0.5f, 1, 0.5f, 1), ALIGN_LEFT, "%s", isServer ? "Areas Exorcised" : "Objectives");
	mSpriteManager->DrawSprite(SPRITESHEET_BARS, 3, anchor + vec2f(50, 6), vec2f(-250, 38), vec4f(0.84f, 1, 0.4f, 1));
	for each (DominationPoint& dp in Factory<DominationPoint>()) {
		if ((dp.mTier == 0 && gameState <= GAME_STATE_CAPTURE_0) || (dp.mTier == 1 && gameState >= GAME_STATE_CAPTURE_1)) {
			mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 13, anchor + vec2f(10, 30.0f * ++i), dp.mController->isDominated ? vec4f(0.7f, 0.7f, 0.7f, 1) : vec4f(1, 1, 1, 1), ALIGN_LEFT, dp.mName);
			mSpriteManager->DrawSprite(SPRITESHEET_CONTROL_ICONS, dp.mController->isDominated ? 11 : 10, anchor + vec2f(150, 30.0f * i + 6) + vec2f(0, 0), vec2f(16, 16), dp.mController->isDominated ? vec4f(0.7f, 0.7f, 0.7f, 1) : vec4f(1, 1, 1, 1));
		}
	}
}

void SkillBar::RenderEndScreen(bool ghostWins)
{
	BlockGame(true);

	mSpriteManager->DrawSprite(SPRITESHEET_CONTROL_ICONS, 15, mSpriteManager->perc2f(0.5f, 0.5f), mSpriteManager->perc2f(1, 1), vec4f(0.7f, 0.7f, 0.7f, 0.5f));

	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 75, mSpriteManager->perc2f(0.5f, 0.4f), vec4f(0.55f, 0.27f, 0.69f, 1), ALIGN_CENTER, "%s", ghostWins ? "Ghost Wins!" : "Explorers Win!");
}

void SkillBar::BlockGame(bool block)
{
	for each (auto& c in Factory<ExplorerController>())
	{
		if ((static_cast<Explorer *>(c.mSceneObject))->mNetworkID->mHasAuthority)
		c.mIsActive = !block;
	}
	for each (auto& c in Factory<GhostController>())
	{
		if ((static_cast<Ghost *>(c.mSceneObject))->mNetworkID->mHasAuthority)
		c.mIsActive = !block;
	}
}