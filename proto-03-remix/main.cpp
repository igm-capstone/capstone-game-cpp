#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\LinearAllocator.h"
#include "Rig3D\MeshLibrary.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>

using namespace Rig3D;

class Proto_03_Remix : public IScene, public virtual IRendererDelegate
{
public:
	Proto_03_Remix()
	{
		mOptions.mWindowCaption = "Key Frame Sample";
		mOptions.mWindowWidth = 800;
		mOptions.mWindowHeight = 600;
		mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen = false;
	}

	~Proto_03_Remix()
	{
		
	}

	 void VInitialize() override {}
	void VUpdate(double milliseconds) override {}
	void VRender() override {}
	void VShutdown() override{}
	void VOnResize() override{}
};

DECLARE_MAIN(Proto_03_Remix);