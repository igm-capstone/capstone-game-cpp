#pragma once
#include "Rig3D/Parametric.h"
#include "BaseComponent.h"

typedef int(*InteresectOBB)(class BaseColliderComponent*, class OrientedBoxColliderComponent*);
typedef int(*InteresectSphere)(class BaseColliderComponent*, class SphereColliderComponent*);


enum CLayer : short
{
	COLLISION_LAYER_ROOT = -2,
	COLLISION_LAYER_QUADRANT,
	COLLISION_LAYER_FLOOR,
	COLLISION_LAYER_WALL,
	COLLISION_LAYER_EXPLORER,
	COLLISION_LAYER_MINION,
	COLLISION_LAYER_SKILL,
	COLLISION_LAYER_LAMP,
	COLLISION_LAYER_DOOR
};

class BaseColliderComponent :
	public BaseComponent
{
public:
	InteresectOBB	 mOnObbTest;
	InteresectSphere mOnSphereTest;
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
	BaseColliderComponent() : mOnObbTest(nullptr), mOnSphereTest(nullptr), mOffset(0.0f), mLayer(COLLISION_LAYER_ROOT), mIsDynamic(true), mIsTrigger(false) {};
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
};

class PlaneColliderComponent :
	public BaseColliderComponent
{
public:
	PlaneCollider mCollider;

	PlaneColliderComponent();
	~PlaneColliderComponent();
};