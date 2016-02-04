#include "stdafx.h"
#include "Skill.h"
#include <trace.h>

SkillBinding& SkillBinding::Set(MouseButton value)
{
	bindingType = bindingType | MOUSE_BUTTON;
	mouseButton = value;
	return *this;
}

SkillBinding& SkillBinding::Set(GamepadButton value)
{
	bindingType = bindingType | GAMEPAD_BUTTON;
	gamepadButton = value;
	return *this;
}

SkillBinding& SkillBinding::Set(KeyCode value)
{
	bindingType = bindingType | KEY_CODE;
	keyCode = value;
	return *this;
}

Skill::Skill() :
	mCoolDownMs(0),
	mDurationMs(0),
	mLastUsed(0),
	mCallback(nullptr)
{
	Engine& engine = Singleton<Engine>::SharedInstance();
	mInput = engine.GetInput();
	mTimer = engine.GetTimer();
}

Skill::~Skill()
{
}

void Skill::SetBinding(SkillBinding& binding)
{
	mBinding = binding;
}

void Skill::Setup(float cooldown, float duration, UseCallback callback)
{
	mCoolDownMs = cooldown * 1000;
	mDurationMs = duration * 1000;
	RegisterUseCallback(callback);
}

void Skill::Update()
{
	bool useSkill = false;

	useSkill = useSkill || (mBinding.bindingType & KEY_CODE)       && mInput->GetKeyDown(mBinding.keyCode);
	useSkill = useSkill || (mBinding.bindingType & MOUSE_BUTTON)   && mInput->GetMouseButtonDown(mBinding.mouseButton);
	useSkill = useSkill || (mBinding.bindingType & GAMEPAD_BUTTON) && mInput->GetGamepadButtonDown(mBinding.gamepadButton);


	if (!useSkill)
	{
		return;
	}

	float appTime = mTimer->GetApplicationTime();
	float timeFromLastUse = appTime - mLastUsed;

	if (timeFromLastUse < mCoolDownMs)
	{
		// on cooldown
		TRACE_WARN("Skill on cooldown: " << (mCoolDownMs - timeFromLastUse) * 0.001f);
		return;
	}

	OnUse(mDurationMs, nullptr, vec3f());
	mLastUsed = appTime;
}