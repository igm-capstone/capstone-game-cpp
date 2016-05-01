#include "stdafx.h"
#include "UIManager.h"
#include "SpriteManager.h"
#include <Scenes/Level01.h>
#include <SceneObjects/DominationPoint.h>
#include <SceneObjects/Ghost.h>
#include <Components/NetworkID.h>
#include <Components/GhostController.h>


UIManager::UIManager()
{
	mSpriteManager = &Rig3D::Singleton<SpriteManager>::SharedInstance();
	mInput = Singleton<Engine>::SharedInstance().GetInput();

	for (int i = 0; i < sizeof(mReadyState); i++) mReadyState[i] = false;
}


UIManager::~UIManager()
{
}

void UIManager::RenderPanel()
{
	mSpriteManager->DrawSprite(SPRITESHEET_PANELS, numBtns - 1, mSpriteManager->perc2f(0.5f, 0) + vec2f(0,64), vec2f(512, 128));

	for (auto i = 0; i < numBtns; i++)
	{
		RenderButton(&mButtons[i], mSpriteManager->perc2f(0.5f, 0) + vec2f(-64.0f * (numBtns-1) + 128.0f * i, 64));
	}
}

void UIManager::RenderButton(Button* b, vec2f pos)
{
	mSpriteManager->DrawSprite(b->sheetID, 8+b->spriteID, pos, vec2f(100, 100));
	mSpriteManager->DrawSprite(b->sheetID, b->spriteID, pos, vec2f(100, 100), vec4f(1,1,1,1), vec3f(1,1), 2*PI * b->skill->Recharged());
	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 16, pos + vec2f(0,30), vec4f(1, 1, 1, 1), ALIGN_CENTER, "%s", b->skill->mName);
	if (b->skill->mCost) mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 25, pos + vec2f(-25, -43), vec4f(0, 0.61f, 0.88f, 1), ALIGN_CENTER, "%.0f", b->skill->mCost);
	if (b->keySpriteID != -1) mSpriteManager->DrawSprite(SPRITESHEET_CONTROL_ICONS, b->keySpriteID, pos + vec2f(30, -33), vec2f(28, 28));
	if (b->isHighlighted) mSpriteManager->DrawSprite(b->sheetID, 15, pos, vec2f(100, 100));
	
	if (b->toolTipID != -1) {
		AddInteractableArea(pos - vec2f(50, 50), pos + vec2f(50, 50), [this, b]() { RenderToolTip(b); return true; }, false);
	}
	if (b->onClick != nullptr) {
		AddInteractableArea(pos - vec2f(50, 50), pos + vec2f(50, 50), b->onClick, true);
	}
}

void UIManager::RenderToolTip(Button* b)
{
	vec2f pos = mSpriteManager->perc2f(0.5f, 0) + vec2f(0, 128+64*0.6f);

	mSpriteManager->DrawSprite(SPRITESHEET_PANELS, 1, pos, vec2f(512*0.6f, 128 * 0.6f));
	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 16, pos + vec2f(0, 0), vec4f(1, 1, 1, 1), ALIGN_CENTER, "Description %d", b->toolTipID);
}

void UIManager::AddSkill(Skill* skill, SpriteSheetCode sheetID, int spriteID, int keySpriteID, int toolTipID, OnInteractArea onClick)
{
	assert(numBtns < 4);

	mButtons[numBtns].sheetID = sheetID;
	mButtons[numBtns].spriteID = spriteID;
	mButtons[numBtns].skill = skill;
	mButtons[numBtns].keySpriteID = keySpriteID;
	mButtons[numBtns].toolTipID = toolTipID;
	mButtons[numBtns].onClick = onClick;

	numBtns++;
}

void UIManager::AddInteractableArea(vec2f topLeft, vec2f bottonRight, OnInteractArea function, bool isClick)
{
	assert(numAreas < MAX_INTERACTIBLE_AREAS);

	mInteractibleAreas[numAreas].topLeft = topLeft;
	mInteractibleAreas[numAreas].bottonRight = bottonRight;
	mInteractibleAreas[numAreas].mFunction = function;
	mInteractibleAreas[numAreas].isClick = isClick;

	numAreas++;
}

void UIManager::SetActiveSkill(Skill* skill)
{
	for (auto i = 0; i < numBtns; i++)
	{
		mButtons[i].isHighlighted = (mButtons[i].skill == skill);
	}
}

void UIManager::SetReadyState(int playerID, bool isReady)
{
	mReadyState[playerID] = isReady;
}

bool UIManager::IsEveryoneReady()
{	
	bool ret = true;
	for (int i = 0; i < sizeof(mReadyState); i++) ret &= mReadyState[i];

	return ret;
}

void UIManager::RenderManaBar()
{
	auto posYPerc = 0.95f;
	for each (Ghost& ghost in Factory<Ghost>()) {
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 2, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(410, 56), vec4f(1, 1, 1, 1), vec2f(ghost.GetManaPerc(), 1));
		mSpriteManager->DrawSprite(SPRITESHEET_BARS, 0, mSpriteManager->perc2f(0.5f, posYPerc), vec2f(450, 56));
	}
}

void UIManager::RenderObjectives(GameState gameState, bool isServer)
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

void UIManager::RenderEndScreen(bool ghostWins)
{
	BlockGame(true);

	mSpriteManager->DrawSprite(SPRITESHEET_CONTROL_ICONS, 15, mSpriteManager->perc2f(0.5f, 0.5f), mSpriteManager->perc2f(1, 1), vec4f(0.7f, 0.7f, 0.7f, 0.5f));

	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 75, mSpriteManager->perc2f(0.5f, 0.4f), vec4f(0.55f, 0.27f, 0.69f, 1), ALIGN_CENTER, "%s", ghostWins ? "Ghost Wins!" : "Explorers Win!");
}

void UIManager::RenderReadyScreen(int playerID)
{
	BlockGame(true);

	mSpriteManager->DrawSprite(SPRITESHEET_CONTROL_ICONS, 15, mSpriteManager->perc2f(0.5f, 0.5f), mSpriteManager->perc2f(1, 1), vec4f(0.7f, 0.7f, 0.7f, 0.5f));
	
	mSpriteManager->DrawSprite(SpriteSheetCode(SPRITESHEET_START_0+playerID), 0, mSpriteManager->perc2f(0.5f, 0.5f), mSpriteManager->perc2f(0.5f, 0.5f) * vec2f(1, 16.0f/11.0f));

	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, 0, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f(-135, 0), vec2f(70, 70));
	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, 1, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f(-45, 0), vec2f(70, 70));
	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, 2, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f(+45, 0), vec2f(70, 70));
	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, 3, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f(+135, 0), vec2f(70, 70));

	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, mReadyState[0] ? 4:5, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f(-135, 0), vec2f(72, 72));
	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, mReadyState[1] ? 4:5, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f( -45, 0), vec2f(72, 72));
	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, mReadyState[2] ? 4:5, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f( +45, 0), vec2f(72, 72));
	mSpriteManager->DrawSprite(SPRITESHEET_GENERAL_ICONS, mReadyState[3] ? 4:5, mSpriteManager->perc2f(0.5f, 0.91f) + vec2f(+135, 0), vec2f(72, 72));

	mSpriteManager->DrawTextSprite(SPRITESHEET_FONT_NORMAL, 20, mSpriteManager->perc2f(0.5f, 0.84f), vec4f(0.1f, 0.1f, 0.1f, 1), ALIGN_CENTER, "Press SPACE when you are READY!");
}

void UIManager::BlockGame(bool block)
{
	for each (auto& c in Factory<ExplorerController>())
	{
		Explorer* e = static_cast<Explorer *>(c.mSceneObject);
		if (e->mNetworkID->mHasAuthority) {
			c.mIsActive = !block;
			for (auto& skill : e->mSkills)
			{
				if (skill) skill->mIsActive = !block;
			}
		}
	}
	for each (auto& c in Factory<GhostController>())
	{
		Ghost* g = static_cast<Ghost *>(c.mSceneObject);
		if (g->mNetworkID->mHasAuthority) {
			c.mIsActive = !block;
			for (auto& skill : g->mSkills)
			{
				if (skill) skill->mIsActive = !block;
			}
		}
	}
}

void UIManager::Update(double ms)
{
	static double hoveringTime;
	bool didHover = false;
	
	mInput->SetMouseActive(true);
	auto mousePos = mInput->mousePosition;

	for (int i = 0; i < numAreas; i++)
	{
		auto area = mInteractibleAreas[i];
		if (mousePos.x >= area.topLeft.x &&
			mousePos.x <= area.bottonRight.x &&
			mousePos.y >= area.topLeft.y &&
			mousePos.y <= area.bottonRight.y &&
			area.mFunction)
		{
			if (area.isClick && mInput->GetMouseButtonDown(MOUSEBUTTON_LEFT)) {
				area.mFunction();
				mInput->SetMouseActive(false);
			}
			else if (!area.isClick && hoveringTime > 500) // 500 ms before hover function gets called
				area.mFunction();
			//Hover delay
			if (!area.isClick && !didHover) {
				hoveringTime += ms;
				didHover = true;
			}
		}
	}
	if (!didHover) hoveringTime = 0;
	numAreas = 0;
}
