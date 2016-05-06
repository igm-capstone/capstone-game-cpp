#pragma once
#include "Rig3D/Parametric.h"
#include "BaseComponent.h"

typedef int(*InteresectOBB)(class BaseColliderComponent*, class OrientedBoxColliderComponent*);
typedef int(*InteresectSphere)(class BaseColliderComponent*, class SphereColliderComponent*);
typedef int(*IntersectRay)(Ray<vec3f>, class BaseColliderComponent*, vec3f& poi, float& t);

enum CLayer : short
{
	COLLISION_LAYER_ROOT = -2,
	COLLISION_LAYER_QUADRANT,
	COLLISION_LAYER_EXPLORER,
	COLLISION_LAYER_INTERACTABLE,
	COLLISION_LAYER_FLOOR,
	COLLISION_LAYER_WALL,
	COLLISION_LAYER_MINION,
	COLLISION_LAYER_EXPLORER_SKILL,
	COLLISION_LAYER_MINION_SKILL
};

class BaseColliderComponent :
	public BaseComponent
{
public:
	InteresectOBB	 mOnObbTest;
	InteresectSphere mOnSphereTest;
	IntersectRay mOnRayTest;

	vec3f	mOffset;
	CLayer	mLayer;
	bool	mIsDynamic;
	bool	mIsTrigger;

	EXPOSE_CALLBACK_2(CollisionEnter, BaseSceneObject*, vec3f)
	EXPOSE_CALLBACK_1(CollisionExit, BaseSceneObject*)

	EXPOSE_CALLBACK_1(TriggerEnter, BaseSceneObject*)
	EXPOSE_CALLBACK_1(TriggerStay, BaseSceneObject*)
	EXPOSE_CALLBACK_1(TriggerExit, BaseSceneObject*)

protected:
	BaseColliderComponent() : mOnObbTest(nullptr), mOnSphereTest(nullptr), mOnRayTest(nullptr), mOffset(0.0f), mLayer(COLLISION_LAYER_ROOT), mIsDynamic(true), mIsTrigger(false) {};
	~BaseColliderComponent() {};
};

class BoxColliderComponent :
	public BaseColliderComponent
{
public:
	BoxCollider		mCollider;

	BoxColliderComponent();
	~BoxColliderComponent();

	static int IntersectAABBOBB(class BaseColliderComponent*, class OrientedBoxColliderComponent*);
	static int IntersectAABBSphere(class BaseColliderComponent*, class SphereColliderComponent*);
	static int IntersectRayAABB(Ray<vec3f>, class BaseColliderComponent*, vec3f&, float&);
};

class OrientedBoxColliderComponent : 
	public BaseColliderComponent
{
public:
	OrientedBoxCollider mCollider;

	OrientedBoxColliderComponent();
	~OrientedBoxColliderComponent();

	static int IntersectOBBOBB(class BaseColliderComponent*, class OrientedBoxColliderComponent*);
	static int IntersectOBBSphere(class BaseColliderComponent*, class SphereColliderComponent*);
	static int IntersectRayOBB(Ray<vec3f>, class BaseColliderComponent*, vec3f&, float&);
};

class QuadColliderComponent :
	public BaseColliderComponent
{
public:
	BoxCollider2D mCollider;

	QuadColliderComponent();
	~QuadColliderComponent();
};

class SphereColliderComponent : 
	public BaseColliderComponent
{
public:
	SphereCollider mCollider;

	SphereColliderComponent();
	~SphereColliderComponent();

	static int IntersectSphereOBB(class BaseColliderComponent*, class OrientedBoxColliderComponent*);
	static int IntersectSphereSphere(class BaseColliderComponent*, class SphereColliderComponent*);
	static int IntersectRaySphere(Ray<vec3f>, class BaseColliderComponent*, vec3f&, float&);
};

class PlaneColliderComponent :
	public BaseColliderComponent
{
public:
	PlaneCollider mCollider;

	PlaneColliderComponent();
	~PlaneColliderComponent();
};