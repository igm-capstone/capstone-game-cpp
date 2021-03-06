#include "stdafx.h"
#include "BaseScene.h"
#include "Rig3D/Graphics/DirectX11/imgui/imgui.h"
#include <SceneObjects/SpawnPoint.h>
#include <SceneObjects/Explorer.h>
#include <SceneObjects/Ghost.h>
#include <SceneObjects/Minion.h>
#include <Rig3D/Graphics/DirectX11/DX11IMGUI.h>
#include <Console.h>
#include <ScareTacticsApplication.h>
#include <Components/MinionController.h>
#include <Rig3D/Graphics/DirectX11/imgui/imgui_internal.h>
#include <UI/SpriteManager.h>

BaseScene::BaseScene() : 
	mStaticMemory(nullptr),
	mStaticMemorySize(0),
	mState(BASE_SCENE_STATE_CONSTRUCTED)
{
	mEngine = &Singleton<Engine>::SharedInstance();

	mRenderer = mEngine->GetRenderer();
	mDevice = mRenderer->GetDevice();
	mDeviceContext = mRenderer->GetDeviceContext();
	mRenderer->SetDelegate(this);

	mInput = mEngine->GetInput();

	mCameraManager = &Singleton<CameraManager>::SharedInstance();
	mNetworkManager = &Singleton<NetworkManager>::SharedInstance();
	mCollisionManager = &Singleton<CollisionManager>::SharedInstance();
	mAIManager = &Singleton<AIManager>::SharedInstance();
	mModelManager = Application::SharedInstance().GetModelManager();
	mSpriteManager = &Singleton<SpriteManager>::SharedInstance();
}

BaseScene::~BaseScene()
{
	// MAKE SURE TO FREE MEMORY HERE OR BEFORE THIS IS CALLED!
}

void BaseScene::SetStaticMemory(void* staticMemory, size_t size)
{
	mStaticMemory = reinterpret_cast<uint8_t*>(staticMemory);
	mStaticMemorySize = size;
}

void BaseScene::RenderFPSIndicator()
{
	ImGui::SetNextWindowPos(ImVec2(mRenderer->GetWindowWidth() - 100.0f, 10.0f), ImGuiSetCond_Always);
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("%6.1f FPS ", ImGui::GetIO().Framerate);
	ImGui::End();
}

void GetChildAndPrint(int index)
{
	auto bvh = &Singleton<CollisionManager>::SharedInstance().mBVHTree;

	for (auto i = 0U; i < bvh->mNodes.size(); i++)
	{
		auto n = bvh->mNodes[i];
		if (n.parentIndex == index) {
			string s = "BVH Node (%d)";
			if (n.object->mSceneObject && n.object->mSceneObject->mClassName)
			{
				s = n.object->mSceneObject->mClassName;
				s.replace(0, 6, "");
				s.append(" (%d)");
			}
			
			if (ImGui::TreeNode((void*)(intptr_t)i, s.c_str(), i)) {
				GetChildAndPrint(i);
				ImGui::TreePop();
			}
		}
	}
}

void BaseScene::RenderBVHTree()
{
	auto bvh = &Singleton<CollisionManager>::SharedInstance().mBVHTree;
	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiSetCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiSetCond_Appearing);
	ImGui::Begin("BVH Tree", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	
	if (ImGui::TreeNode((void*)(intptr_t)0, "Root")) {
		GetChildAndPrint(0);
		ImGui::TreePop();
	}
	
	ImGui::End();
}

void BaseScene::RenderMinionBehaviorTrees()
{
	//ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiSetCond_Always);
	//ImGui::SetNextWindowContentSize(ImVec2(200.0f, 600.0f));
	ImGui::Begin("Minion Behavior Trees", nullptr, ImGuiWindowFlags_NoCollapse);

	for (Minion& minion : Factory<Minion>())
	{
		int id = 0;
		minion.mController->mBehaviorTree->DumpIMGUI(id);
	}

	ImGui::End();
}

/* Renders IMGUI. ideally, it is the last call in the render loop.
 * Optionally takes a void(*)(BaseScene*) function that draw custom IMGUI on top of the default FPS and Console. */
void BaseScene::RenderIMGUI(void(*IMGUIDrawFunc)(BaseScene*))
{
	mRenderer->VSetContextTarget();
	DX11IMGUI::NewFrame();
	RenderFPSIndicator();
#ifdef _DEBUG
	if (gDebugBVH) RenderBVHTree();
	if (gDebugBT) RenderMinionBehaviorTrees();
	RENDER_TRACE_WATCH();
#endif
	if (IMGUIDrawFunc) IMGUIDrawFunc(this);
	ImGuiWindow* console = static_cast<ImGuiWindow*>(Console::Draw());
	ImGui::Render();

	ImGuiState* state = static_cast<ImGuiState*>(ImGui::GetInternalState());
	ImGuiWindow* root = state->HoveredRootWindow;
	mInput->SetMouseActive(root == nullptr || !Console::IsVisible() && root == console);
}