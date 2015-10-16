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
#include "PathFinder\Graph.h"
#include "Fringe.h"

using namespace Rig3D;

static const int VERTEX_COUNT = 8;
static const int INDEX_COUNT = 36;
static const int KEY_FRAME_COUNT = 10;

class Proto_03_Remix : public IScene, public virtual IRendererDelegate
{
	PathFinder::Graph<10, 10> graph;

public:
	Proto_03_Remix()
	{
		mOptions.mWindowCaption = "Key Frame Sample";
		mOptions.mWindowWidth = 800;
		mOptions.mWindowHeight = 600;
		mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen = false;
	}

	~Proto_03_Remix() {}

	void VInitialize() override
	{
		graph = PathFinder::Graph<10, 10>();

		graph.grid[4][8].weight = 100;
		graph.grid[4][7].weight = 100;
		graph.grid[4][6].weight = 100;
		graph.grid[4][5].weight = 100;
		graph.grid[4][4].weight = 100;
		graph.grid[4][3].weight = 100;
	}

	void VUpdate(double milliseconds) override {


		if ((&Input::SharedInstance())->GetKeyDown(KEYCODE_UP))
		{
			TRACE("Treta" << 1 << " " << 1.0f << true);

			auto search = PathFinder::Fringe<10, 10>(graph);

			auto start = &graph.grid[1][5];
			auto end = &graph.grid[8][5];

			auto result = search.FindPath(start, end);

			std::stringstream ss;
			for (int y = 0; y < 10; y++)
			{
				for (int x = 0; x < 10; x++)
				{
					bool inPath = false;
					for (auto it = result.path.begin(); it != result.path.end(); ++it)
					{
						if (**it == graph.grid[x][y])
						{
							inPath = true;
							break;
						}
					}

					ss << " " << (inPath ? 'X' : graph.grid[x][y].weight > 1 ? '#' : 'O');
				}
				ss << std::endl;
			}

			TRACE(ss.str());

		}
	}

	void VRender() override{}
	void VShutdown() override{}
	void VOnResize() override{}
};

DECLARE_MAIN(Proto_03_Remix);