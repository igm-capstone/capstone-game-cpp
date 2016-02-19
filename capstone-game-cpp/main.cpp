#include "stdafx.h"
#include "Rig3D\Graphics\Interface\IScene.h"

//#include "Scenes\MainMenu.h"
//#include "Scenes\Credits.h"
#include <capstone-game-cpp/ScareTacticsApplication.h>
#include "Scenes\LoadingScreen.h"
#include "Scenes\MainMenuScene.h"
#include "capstone-game-cpp.rc.h"

#define STATIC_APP_MEMORY 4000000

static char gApplicationMemory[STATIC_APP_MEMORY];	// 4mb

Rig3D::IScene *gRig3DScene = nullptr;

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR cmdLine,
	int showCmd)
{
	using namespace Rig3D;
	
	if (strcmp(cmdLine, "--local-client") == 0)
	{
		MainMenuScene::gLocalClient = true;
	}
	else if (strcmp(cmdLine, "--local-server") == 0)
	{
		MainMenuScene::gLocalServer = true;
	}
	else if (strcmp(cmdLine, "--local-all") == 0)
	{
		MainMenuScene::gLocalClient = true;
		MainMenuScene::gLocalServer = true;
	}
	else if (strcmp(cmdLine, "--local-debug") == 0)
	{
		MainMenuScene::gDebugBoth = true;
	}

	struct stat s;
	if (stat("Assets", &s) != 0) {
		_wchdir((LPCWSTR)L"..");
		if (stat("Assets", &s) != 0) {
			_wchdir((LPCWSTR)L"..");
			if (stat("Assets", &s) != 0) {
				MessageBox(NULL, (LPCWSTR)L"Assets folder not found.", (LPCWSTR)L"Error", MB_ICONERROR | MB_OK);
				exit(1);
			}
		}
	}

	Engine& engine = Singleton<Engine>::SharedInstance();

	Options options;
	options.mWindowCaption = "Scare Tactics";
	options.mWindowWidth = 1600;
	options.mWindowHeight = 1000;
	options.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	options.mFullScreen = false;

	engine.Initialize(hInstance,
		hPrevInstance,
		cmdLine,
		showCmd,
		options);

	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(engine.GetRenderer()->GetHWND(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	Application* app = &Application::SharedInstance();
	app->mOptions = options;

	LoadingScreen loading;
	app->SetLoadingScreen(&loading);
	
	app->SetStaticMemory(gApplicationMemory, STATIC_APP_MEMORY);
	app->LoadScene<MainMenuScene>();

	engine.RunApplication(app);

	return 0;
}