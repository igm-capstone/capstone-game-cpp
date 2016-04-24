#pragma once
#include <Components/Skill.h>
#include <UI/SpriteManager.h>

enum GameState : char;

typedef std::function<bool(void)> OnInteractArea;

struct Button
{
	bool isHighlighted = false;
	SpriteSheetCode sheetID;
	int spriteID;
	int keySpriteID;
	int toolTipID;
	OnInteractArea onClick;
	Skill* skill;
};

struct InteractableArea
{
	vec2f topLeft;
	vec2f bottonRight;
	OnInteractArea mFunction = nullptr;
	bool isClick = false;
};

class UIManager
{
	SpriteManager* mSpriteManager;
	Button mButtons[4];
	int numBtns = 0;
	InteractableArea mInteractibleAreas[MAX_INTERACTIBLE_AREAS];
	int numAreas = 0;

	bool mReadyState[MAX_PLAYERS];

	Input* mInput;

	void AddInteractableArea(vec2f topLeft, vec2f bottonRight, OnInteractArea function = nullptr, bool isClick = false);

public:
	UIManager();
	~UIManager();

	void RenderPanel();
	void RenderButton(Button* b, vec2f pos);
	void RenderToolTip(Button* b);
	void AddSkill(Skill* name, SpriteSheetCode sheetID, int spriteID, int keySpriteID = -1, int toolTipID = -1, bool canClickToSetActive = false);
	void AddButton(vec2f pos, vec2f size, char* text, OnInteractArea function = false);
	void SetActiveSkill(Skill* skill);
	void SetReadyState(int playerID, bool isReady);
	bool GetReadyState(int playerID) { return mReadyState[playerID]; };
	bool IsEveryoneReady();
	void RenderManaBar();
	void RenderObjectives(GameState gameState, bool isServer);
	void RenderEndScreen(bool ghostWins);
	void RenderReadyScreen(int playerID);
	void BlockGame(bool block);
	void Update(double ms);
};

