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

class SkillBar
{
	SpriteManager* mSpriteManager;
	Button mButtons[4];
	int numBtns = 0;

public:
	SkillBar();
	~SkillBar();

	void RenderPanel();
	void RenderButton(Button* b, vec2f pos);
	void AddSkill(Skill* name, SpriteSheetCode sheetID, int spriteID, int keySpriteID = -1);
	void SetActive(Skill* skill);
	void RenderManaBar();
	void RenderObjectives(GameState gameState, bool isServer);
};

