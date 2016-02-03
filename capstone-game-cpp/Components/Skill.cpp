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

class colWrapper
{
public:
	colWrapper(BoxCollider* c) : mCollider(*c) {}

	BoxCollider& mCollider;
};

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

	auto scene = Application::SharedInstance().GetCurrentScene();
	auto camera = scene->mCamera;
	auto renderer = Singleton<Engine>::SharedInstance().GetRenderer();
	auto mousePosition = mInput->mousePosition;

	float x = (2.0f * mousePosition.x) / renderer->GetWindowWidth() - 1.0f;
	float y = 1.0f - (2.0f * mousePosition.y) / renderer->GetWindowHeight();

	vec4f rayClip(x, y, 0, 1);
	vec4f rayEye = rayClip * camera.GetProjectionMatrix().inverse();
	rayEye.z = 0;
	rayEye.w = 0;

	vec3f rayWorld = rayEye * camera.GetViewMatrix().inverse();



	//float x = (2.0f * mousePosition.x) / renderer->GetWindowWidth() - 1.0f;
	//float y = 1.0f - (2.0f * mousePosition.y) / renderer->GetWindowHeight();

	mat4f toWorld = (camera.GetProjectionMatrix() * camera.GetViewMatrix()).inverse();
	vec3f worldPos = vec4f(x, y, 0.0f, 1.0f) * toWorld;
	vec3f worldDir = camera.GetForward();
	worldPos.z = 10.0f;


	vec3f skillPos {};
	RayCastHit<vec3f> hit;
	//Ray<vec3f> ray = { normalize(rayWorld), camera.GetForward() };
	Ray<vec3f> ray = { worldPos, worldDir };
	//// WTF BRO? HOW DO I DO DIS

	if (RayCast(&hit, ray, &colWrapper(&scene->mFloorCollider), 1))
	{
		skillPos = hit.poi;
	}
	
	TRACE_LOG(x << " " << y);
	TRACE_LINE(worldPos, hit.poi, Colors::red);
	TRACE_BOX(worldPos, Colors::blue);
	TRACE_BOX(hit.poi,  Colors::green);
	worldPos.z = 0;
	TRACE_BOX(vec3f(),  Colors::red);


	if (!useSkill)
	{
		return;
	}

	OnUse(mDurationMs, nullptr, skillPos);

	mLastUsed = appTime;
}