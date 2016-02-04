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
	BindingType   bindingType = BindingType(0);
	MouseButton   mouseButton = MouseButton(0);
	GamepadButton gamepadButton = GamepadButton(0);
	KeyCode       keyCode = KeyCode(0);

	SkillBinding& Set(MouseButton value);
	SkillBinding& Set(GamepadButton value);
	SkillBinding& Set(KeyCode value);
};

class Skill : public BaseComponent
{
	friend class Factory<Skill>;

public:
	typedef void(*SkillCallback)(Skill*, BaseSceneObject*, vec3f);

private:
	Input* mInput;

	SkillCallback mCallback;
	SkillBinding  mBinding;

	float mCoolDown;
	float mDuration;

	Skill();
	~Skill();

public:
	void SetBinding(SkillBinding& binding);
	void Setup(float cooldown, float duration, SkillCallback callback);
	void Update();
};

inline BindingType operator|(BindingType a, BindingType b)
{
	return static_cast<BindingType>(static_cast<int>(a) | static_cast<int>(b));
}
