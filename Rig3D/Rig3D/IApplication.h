#pragma once
#include "Graphics/Interface/IScene.h"
#include "Memory/Memory/Memory.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D IApplication
	{
	public:
		Options mOptions;

		virtual void VInitialize() = 0;
		virtual void VUpdateCurrentScene() = 0;
		virtual void VUpdate(float deltaTime) = 0;
		virtual void VShutdown() = 0;

	protected:
		IApplication()
		{
			
		}
		
		virtual ~IApplication()
		{
			
		}

		IApplication(IApplication const&) = delete;
		void operator=(IApplication const&) = delete;
	};
}