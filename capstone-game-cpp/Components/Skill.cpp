#include "stdafx.h"
#include "Skill.h"

SkillBinding& SkillBinding::Set(MouseButton value)
{
	bindingType = bindingType | BindingType::MOUSE_BUTTON;
	mouseButton = value;
	return *this;
}

SkillBinding& SkillBinding::Set(GamepadButton value)
{
	bindingType = bindingType | BindingType::GAMEPAD_BUTTON;
	gamepadButton = value;
	return *this;
}

SkillBinding& SkillBinding::Set(KeyCode value)
{
	bindingType = bindingType | BindingType::KEY_CODE;
	keyCode = value;
	return *this;
}

Skill::Skill():
	mInput(Singleton<Engine>::SharedInstance().GetInput()),
	mCallback(nullptr),
	mCoolDown(0),
	mDuration(0)
{
		
}

Skill::~Skill()
{
}

void Skill::SetBinding(SkillBinding& binding)
{
	mBinding = binding;
}

void Skill::Setup(float cooldown, float duration, SkillCallback callback)
{
	mCoolDown = cooldown;
	mDuration = duration;
	mCallback = callback;
}

void Skill::Update()
{
	bool useSkill = false;

	useSkill = useSkill || (mBinding.bindingType & KEY_CODE)       && mInput->GetKeyDown(mBinding.keyCode);
	useSkill = useSkill || (mBinding.bindingType & MOUSE_BUTTON)   && mInput->GetMouseButtonDown(mBinding.mouseButton);
	useSkill = useSkill || (mBinding.bindingType & GAMEPAD_BUTTON) && mInput->GetGamepadButtonDown(mBinding.gamepadButton);

	if (mCallback && useSkill)
	{
		mCallback(this, nullptr, vec3f());
	}
}