#pragma once
#include <Scenes/BaseScene.h>

struct Button
{
	bool isHighlighted = false;
	int sheetID;
	int spriteID;
	const char* name;
};

class SkillBar
{
	SpriteManager* mSpriteManager;
	Button mButtons[4];

public:
	SkillBar();
	~SkillBar();

	void RenderPanel();
	void RenderButton();
	void AddSkill(const char* name, int sheetID, int spriteID);
};

