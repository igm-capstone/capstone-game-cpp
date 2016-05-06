#pragma once
#include "BaseComponent.h"

enum BindingType
{
	KEY_CODE       = 1 << 0,
	MOUSE_BUTTON   = 1 << 1,
	GAMEPAD_BUTTON = 1 << 2,
};

struct SkillBinding
{
	BindingType   bindingType   = BindingType(0);
	MouseButton   mouseButton   = MouseButton(0);
	GamepadButton gamepadButton = GamepadButton(0);
	KeyCode       keyCode       = KeyCode(0);

	SkillBinding();
	SkillBinding(SkillBinding&);
	SkillBinding(MouseButton value);
	SkillBinding(GamepadButton value);
	SkillBinding(KeyCode value);

	SkillBinding& Set(MouseButton value);
	SkillBinding& Set(GamepadButton value);
	SkillBinding& Set(KeyCode value);
};

class Skill : public BaseComponent
{
	friend class Factory<Skill>;
	EXPOSE_CALLBACK_BOOL_3(Use, float, BaseSceneObject*, vec3f)

public:
	vec3f mColliderOffset;
	float mCoolDown;
	float mCost;
	float mDuration;
	float mLastUsed;
	char mName[15];
	
	UseCallback mCallback;
	SkillBinding  mBinding;

private:
	Input* mInput;
	Timer* mTimer;

	Skill();
	~Skill();


public:
	void SetBinding(SkillBinding binding);
	void Setup(std::string name, float cooldown, float duration, UseCallback callback, float cost = 0);
	void Update();
	bool UseSkill(BaseSceneObject* target = nullptr, vec3f skillPos = vec3f(0,0,0));
	float Recharged();
};

inline BindingType operator|(BindingType a, BindingType b)
{
	return static_cast<BindingType>(static_cast<int>(a) | static_cast<int>(b));
}
