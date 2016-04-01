#include "stdafx.h"
#include "Skill.h"
#include <trace.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Parametric.h>
#include <RayCast.h>
#include <Colors.h>

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
	mCoolDown = cooldown;
	mDuration = duration;
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
	if (!mIsActive)
	{
		return;
	}

	//--- check if a binding was activated

	bool useSkill = false;

	useSkill = useSkill || (mBinding.bindingType & KEY_CODE)       && mInput->GetKeyDown(mBinding.keyCode);
	useSkill = useSkill || (mBinding.bindingType & MOUSE_BUTTON)   && mInput->GetMouseButtonDown(mBinding.mouseButton);
	useSkill = useSkill || (mBinding.bindingType & GAMEPAD_BUTTON) && mInput->GetGamepadButtonDown(mBinding.gamepadButton);

	if (!useSkill)
	{
		return;
	}

	//--- check if skill is still on cooldown

	// app time in seconds
	float appTime = float(mTimer->GetApplicationTime()) * 0.001f;
	float timeFromLastUse = appTime - mLastUsed;

	if (timeFromLastUse < mCoolDown)
	{
		// on cooldown
		TRACE_WARN("Skill on cooldown: " << (mCoolDown - timeFromLastUse));
		return;
	}

	//--- set target
	auto bvhTree = &Singleton<CollisionManager>::SharedInstance();
	auto mCameraManager = &Singleton<CameraManager>::SharedInstance();
	auto ray = mCameraManager->Screen2Ray(mInput->mousePosition);
	vec3f skillPos;

	auto ret =	bvhTree->mBVHTree.RayCastRecursively(ray, skillPos);
	
	//--- fire callback
	OnUse(mDuration, ret ? ret->mSceneObject : nullptr, skillPos);
	mLastUsed = appTime;
}