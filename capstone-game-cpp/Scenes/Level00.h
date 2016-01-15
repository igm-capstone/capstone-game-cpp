#pragma once

#include "PathFinder/Fringe.h"
#include "Rig3D/GraphicsMath/cgm.h"
#include "Rig3D/Intersection.h"
#include "Rig3D/Parametric.h"
#include "SceneObject.h"
#include "LevelReader.h"
#include "Colors.h"
#include "Grid.h"
#include "Vertex.h"
#include "TargetFollower.h"
#include "Rig3D/Application.h"
#include "Network\NetworkManager.h"

//Shaders - Headers are output from compiler
#include "Shaders/obj/BillboardPixelShader.h"
#include "Shaders/obj/BillboardVertexShader.h"
#include "Shaders/obj/CircleVertexShader.h"
#include "Shaders/obj/LineTracePixelShader.h"
#include "Shaders/obj/LineTraceVertexShader.h"
#include "Shaders/obj/QuadPixelShader.h"
#include "Shaders/obj/QuadVertexShader.h"
#include "Shaders/obj/ShadowCasterPixelShader.h"
#include "Shaders/obj/ShadowGridComputeShader.h"
#include "Shaders/obj/ShadowPixelShader.h"

#define PI					3.1415926535f
#define UNITY_QUAD_RADIUS	0.85f

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

class Level00 : public IScene, public virtual IRendererDelegate
{
public:
	struct QuadShaderData
	{
		mat4f View;
		mat4f Projection;
		vec4f Color;
	};

	struct PointShaderData
	{
		vec4f Point;
	};

	struct LineTraceShaderData
	{
		mat4f View;
		mat4f Projection;
	};

	struct LineTraceVertex
	{
		vec4f Color;
		vec3f Position;
	};

	struct GridNode
	{
		vec3f worldPos;
		int x;
		int y;
		float weight;
		bool hasLight;
	};

	GridNode						mGridNodeData[numSpheresX*numSpheresY];

	QuadShaderData					mQuadShaderData;
	LineTraceShaderData				mLineTraceShaderData;
	PointShaderData					mPointShaderData;

	mat4f							mViewMatrix;
	mat4f							mProjectionMatrix;

	Transform						mPlayerTransform;
	Transform						mGoalTransform;

	LinearAllocator					mSceneAllocator;
	LinearAllocator					mStaticMeshAllocator;
	LinearAllocator					mDynamicMeshAllocator; // TO DO: This will likely need to be a Pool Allocator ->Gabe

	MeshLibrary<LinearAllocator>	mStaticMeshLibrary;
	MeshLibrary<LinearAllocator>	mDynamicMeshLibrary;

	SceneObject*					mWalls;
	SceneObject*					mBlocks;
	SceneObject*					mWaypoints;
	SceneObject*					mLights;

	SceneObject						mPlayer;
	SceneObject						mGoal;

	Node*							mPlayerNode;

	BoxCollider						mPlayerCollider;

	vector<RobotInfo>				mRobots;
	TargetFollower*					mFollowers;

	AABB<vec2f>*					mAABBs;
	Sphere<vec3f>*					mLightColliders;

	int								mLineTraceDrawCount;
	bool							mLineTraceOverflow;

	mat4f*							mWallTransforms;
	mat4f*							mBlockTransforms;
	mat4f*							mCircleTransforms;
	mat4f*							mRobotTransforms;
	float*							mCircleColorWeights;

	std::vector<vec3f>				mLightPos;

	BoxCollider*					mWallColliders;

	int								mWallCount;
	int								mBlockCount;
	int								mCircleCount;
	int								mRobotCount;
	int								mAABBCount;

	IMesh*							mWallMesh;
	IMesh*							mRobotMesh;
	IMesh*							mCircleMesh;
	IMesh*							mLightMesh;
	IMesh*							mPlayerMesh;

	Renderer*						mRenderer;

	ID3D11Device*					mDevice;
	ID3D11DeviceContext*			mDeviceContext;

	ID3D11InputLayout*				mQuadInputLayout;
	ID3D11VertexShader*				mQuadVertexShader;
	ID3D11PixelShader*				mQuadPixelShader;
	ID3D11Buffer*					mQuadShaderBuffer;
	ID3D11Buffer*					mWallInstanceBuffer;

	ID3D11InputLayout*				mLineTraceInputLayout;
	ID3D11VertexShader*				mLineTraceVertexShader;
	ID3D11PixelShader*				mLineTracePixelShader;
	ID3D11Buffer*					mLineTraceShaderBuffer;

	ID3D11InputLayout*				mCircleInputLayout;
	ID3D11VertexShader*				mCircleVertexShader;
	ID3D11PixelShader*				mCirclePixelShader;
	ID3D11Buffer*					mCircleInstanceBuffer;
	ID3D11Buffer*					mColorWeightInstanceBuffer;

	ID3D11Buffer*					mPlayerInstanceBuffer;

	ID3D11RenderTargetView*			mShadowCastersRTV;
	ID3D11Texture2D*				mShadowCastersMap;
	ID3D11ShaderResourceView*		mShadowCastersSRV;
	ID3D11RenderTargetView*			mShadowsARTV;
	ID3D11Texture2D*				mShadowsAMap;
	ID3D11ShaderResourceView*		mShadowsASRV;
	ID3D11RenderTargetView*			mShadowsBRTV;
	ID3D11Texture2D*				mShadowsBMap;
	ID3D11ShaderResourceView*		mShadowsBSRV;
	ID3D11RenderTargetView*			mShadowsFinalRTV;
	ID3D11Texture2D*				mShadowsFinalMap;
	ID3D11ShaderResourceView*		mShadowsFinalSRV;
	ID3D11PixelShader*				mShadowCasterPixelShader;
	ID3D11VertexShader*				mBillboardVertexShader;
	ID3D11PixelShader*				mBillboardPixelShader;
	ID3D11PixelShader*				mShadowPixelShader;
	ID3D11SamplerState*				mSamplerState;
	ID3D11BlendState*				mBlendStateShadowMask;
	ID3D11BlendState*				mBlendStateShadowCalc;
	ID3D11Buffer*					mPointShaderBuffer;


	ID3D11ComputeShader*		mShadowGridComputeShader;
	ID3D11Buffer*				mSrcDataGPUBuffer;
	ID3D11ShaderResourceView*	mSrcDataGPUBufferView;

	ID3D11Buffer*				mDestDataGPUBuffer;
	ID3D11Buffer*				mDestDataGPUBufferCPURead;
	ID3D11UnorderedAccessView*	mDestDataGPUBufferView;


	float white[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float transp[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float black[4] = { 0.0f, 0.0f, 0.0f, 0.5f };

	// singletons
	Engine* mEngine;
	Input* mInput;
	Grid& mGrid = Grid::SharedInstance();
	NetworkManager* mNetworkManager;
	ID3D11ShaderResourceView* nullSRV[3] = { 0, 0, 0 };

#if defined _DEBUG
	LineTraceVertex*				mLineTraceVertices;
	IMesh*							mLineTraceMesh;
#endif // defined _DEBUG


	Level00();
	~Level00(){}

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;

	void RenderWalls();
	void RenderLightCircles();
	void RenderShadowMask();
	void RenderPlayer();
	void RenderRobots();
	void RenderGrid();
	void RenderLineTrace();
	void UpdatePlayer();
	void UpdateGrid();
	void UpdateRobots();

	void InitializeCamera();
	void InitializeLevel();
	void InitializeGrid();
	void InitializeRobots();
	void LoadTransforms(mat4f** transforms, vec3f* positions, vec3f* rotations, vec3f* scales, int size, int TransformType);
	void LoadStaticSceneObjects(SceneObject** sceneObjects, mat4f** transforms, BoxCollider** colliders, vec3f* positions, vec3f* rotations, vec3f* scales, int size);
	void LoadLights(SceneObject** sceneObjects, mat4f** transforms, SphereCollider** colliders, vec3f* positions, int size);
	static void SetAABBs(RectInfo rectInfo, AABB<Vector2>* aabb, int offset);
	void InitializeGeometry();
	void InitializeLineTraceMesh();
	void InitializeQuadMesh();
	void InitializeCircleMesh();
	void InitializeLineTraceShaders();
	void InitializeWallShaders();
	void InitializeLightShaders();
	void InitializePlayerShaders();

	void TraceLine(const float& from_x, const float& from_y, const float& from_z, const float& to_x, const float& to_y, const float& to_z, const vec4f& color);

	void HandleInput(Input& input);
};