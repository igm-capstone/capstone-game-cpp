#pragma once
#include <Components/Skill.h>

struct Button
{
	bool isHighlighted = false;
	int sheetID;
	int spriteID;
	Skill* skill;
};

class SkillBar
{
	class SpriteManager* mSpriteManager;
	Button mButtons[4];
	int numBtns = 0;

public:
	SkillBar();
	~SkillBar();

	void RenderPanel();
	void RenderButton(Button* b, vec2f pos);
	void AddSkill(Skill* name, int sheetID, int spriteID);
	void SetActive(Skill* skill);
};

