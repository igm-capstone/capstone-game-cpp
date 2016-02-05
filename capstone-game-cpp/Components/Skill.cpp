#include "stdafx.h"
#include "Skill.h"
#include <trace.h>
#include <ScareTacticsApplication.h>
#include <Rig3D/Parametric.h>
#include <RayCast.h>

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

	auto camera = Application::SharedInstance().GetCurrentScene()->mCamera;
	auto renderer = Singleton<Engine>::SharedInstance().GetRenderer();
	auto mousePosition = mInput->mousePosition;

	float x = (2.0f * mousePosition.x) / renderer->GetWindowWidth() - 1.0f;
	float y = 1.0f - (2.0f * mousePosition.y) / renderer->GetWindowHeight();

	mat4f toWorld = (camera.GetProjectionMatrix() * camera.GetViewMatrix()).inverse();
	vec3f worldPos = vec4f(x, y, 0.0f, 1.0f) * toWorld;
	vec3f worldDir = camera.GetForward();
	worldPos.z = -30.0f;

	vec3f skillPos {};

	RayCastHit<vec3f> hit;
	Ray<vec3f> ray = { worldPos, worldDir };
	/*if (RayCast(&hit, ray, mLightColliders, mCircleCount))
	{
		skillPos = hit.poi;
	}
	*/
	OnUse(mDurationMs, nullptr, skillPos);

	mLastUsed = appTime;
}