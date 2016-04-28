#include "stdafx.h"
#include "Skill.h"
#include <trace.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Parametric.h>
#include <RayCast.h>
#include <Colors.h>

SkillBinding::SkillBinding()
{
}

SkillBinding::SkillBinding(SkillBinding& binding)
{
	mouseButton = binding.mouseButton;
	bindingType = binding.bindingType;
	gamepadButton = binding.gamepadButton;
	keyCode = binding.keyCode;
}


SkillBinding::SkillBinding(MouseButton value)
{
	Set(value);
}

SkillBinding::SkillBinding(GamepadButton value)
{
	Set(value);
}

SkillBinding::SkillBinding(KeyCode value)
{
	Set(value);
}

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
	mCoolDown(0),
	mDuration(0),
	mLastUsed(-FLT_MAX),
	mCallback(nullptr)
{
	Engine& engine = Singleton<Engine>::SharedInstance();
	mInput = engine.GetInput();
	mTimer = engine.GetTimer();
}

Skill::~Skill()
{
}

void Skill::SetBinding(SkillBinding binding)
{
	mBinding = binding;
}

void Skill::Setup(string name, float cooldown, float duration, UseCallback callback, float cost)
{
	auto lastIndex = min(name.size(), sizeof(mName) - 1);
	memcpy(mName, name.c_str(), lastIndex);
	mName[lastIndex] = 0;

	mCoolDown = cooldown;
	mDuration = duration;
	mCost = cost;
	RegisterUseCallback(callback);
}

class colWrapper
{
public:
	colWrapper(BoxCollider* c) : mCollider(*c) {}

	BoxCollider& mCollider;
};

void Skill::Update()
{
	if (!mIsActive) return;

	//--- check if a binding was activated
	bool useSkill = false;
	useSkill = useSkill || (mBinding.bindingType & KEY_CODE)       && mInput->GetKeyDown(mBinding.keyCode);
	useSkill = useSkill || (mBinding.bindingType & MOUSE_BUTTON)   && mInput->GetMouseButtonDown(mBinding.mouseButton);
	useSkill = useSkill || (mBinding.bindingType & GAMEPAD_BUTTON) && mInput->GetGamepadButtonDown(mBinding.gamepadButton);

	if (!useSkill) return;

	BaseSceneObject* target = nullptr;
	vec3f skillPos;

	if ((mBinding.bindingType & MOUSE_BUTTON)) {
		//--- set target
		auto bvhTree = &Singleton<CollisionManager>::SharedInstance();
		auto mCameraManager = &Singleton<CameraManager>::SharedInstance();
		auto ray = mCameraManager->Screen2Ray(mInput->mousePosition);


		auto ret = bvhTree->mBVHTree.RayCastRecursively(ray, skillPos);
		target = ret ? ret->mSceneObject : nullptr;
	}

	UseSkill(target, skillPos);
}

bool Skill::UseSkill(BaseSceneObject* target, vec3f skillPos)
{
	//Cooldown
	float appTime = float(mTimer->GetApplicationTime()) * 0.001f;
	float timeFromLastUse = appTime - mLastUsed;

	if (timeFromLastUse < mCoolDown)
	{
		TRACE_WARN("Skill on cooldown: " << (mCoolDown - timeFromLastUse));
		return false;
	}

	//Fire callback
	if (OnUse(mDuration, target, skillPos)) {
		mLastUsed = appTime;
		return true;
	}

	return false;
}

float Skill::Recharged()
{
	float appTime = float(mTimer->GetApplicationTime()) * 0.001f;
	float timeFromLastUse = appTime - mLastUsed;

	if (timeFromLastUse < mCoolDown) return timeFromLastUse / mCoolDown;
	
	return 1;
}
