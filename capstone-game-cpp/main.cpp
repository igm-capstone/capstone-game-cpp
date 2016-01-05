#include "Rig3D\Graphics\Interface\IScene.h"

//#include "Scenes\MainMenu.h"
//#include "Scenes\Credits.h"
#include "Scenes\Level00.h"


Rig3D::IScene *gRig3DScene = 0;
int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR cmdLine,
	int showCmd)
{
	Level00 scene;
	gRig3DScene = &scene;
	Rig3D::Engine engine = Rig3D::Engine();
	engine.Initialize(hInstance,
		hPrevInstance,
		cmdLine,
		showCmd,
		gRig3DScene->mOptions);
	engine.RunScene(gRig3DScene);
	return 0;
}