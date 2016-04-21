#pragma once
#include <Components/Skill.h>
#include <UI/SpriteManager.h>

struct Button
{
	bool isHighlighted = false;
	SpriteSheetCode sheetID;
	int spriteID;
	int keySpriteID;
	Skill* skill;
};

enum GameState : char;

class UIManager
{
	SpriteManager* mSpriteManager;
	Button mButtons[4];
	int numBtns = 0;

	bool mReadyState[MAX_PLAYERS];

public:
	UIManager();
	~UIManager();

	void RenderPanel();
	void RenderButton(Button* b, vec2f pos);
	void AddSkill(Skill* name, SpriteSheetCode sheetID, int spriteID, int keySpriteID = -1);
	void SetActiveSkill(Skill* skill);
	void SetReadyState(int playerID, bool isReady);
	bool GetReadyState(int playerID) { return mReadyState[playerID]; };
	bool IsEveryoneReady();
	void RenderManaBar();
	void RenderObjectives(GameState gameState, bool isServer);
	void RenderEndScreen(bool ghostWins);
	void RenderReadyScreen(int playerID);
	void BlockGame(bool block);
};

