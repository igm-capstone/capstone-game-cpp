#include "Rig3D\Graphics\Interface\IScene.h"

//#include "Scenes\MainMenu.h"
//#include "Scenes\Credits.h"
#include <Rig3D/Application.h>
//#include "Scenes\Level00.h"
#include "Scenes\LoadingScreen.h"

Rig3D::IScene *gRig3DScene = 0;
int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR cmdLine,
	int showCmd)
{
	Engine& engine = Singleton<Engine>::SharedInstance();

	Options options;
	options.mWindowCaption = "Shutter - Remix";
	options.mWindowWidth = 1600;
	options.mWindowHeight = 1000;
	options.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	options.mFullScreen = false;

	engine.Initialize(hInstance,
		hPrevInstance,
		cmdLine,
		showCmd,
		options);

	LoadingScreen loading = LoadingScreen();

	Application* app = engine.GetApplication();
	app->mLoadingScene = &loading;
	//app.LoadScene<LoadingScreen>();
	//app.mCurrentScene = &loading;

	engine.RunApplication(app);
	return 0;

	//Level00 scene;
	//gRig3DScene = &scene;
	//engine.Initialize(hInstance,
	//	hPrevInstance,
	//	cmdLine,
	//	showCmd,
	//	gRig3DScene->mOptions);
	//engine.RunScene(gRig3DScene);
	return 0;
}