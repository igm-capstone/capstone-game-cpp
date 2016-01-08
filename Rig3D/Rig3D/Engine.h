#pragma once
#include <Windows.h>
#include "Rig3D\Common\WMEventHandler.h"
#include "Rig3D\Common\Timer.h"
#include "Rig3D\Common\Input.h"
#include "Rig3D\rig_defines.h"
#include "Rig3D\Options.h"
#include "Rig3D/Singleton.h"
#include "Rig3D\Graphics\Interface\IRenderer.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	typedef IRenderer<DX3D11Renderer> Renderer;

	class Application;
	class IScene;

	class RIG3D Engine : public virtual IObserver
	{
	public:
		Engine();
		~Engine();

		int		Initialize(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd, Options options);
		void	BeginScene();
		void	EndScene();
		int		Shutdown();
		void	HandleEvent(const IEvent& iEvent) override;

		void	RunScene(IScene* iScene);
		void	RunApplication(Application* app);

		Renderer*		GetRenderer() const;
		Input*			GetInput() const;
		Timer*			GetTimer() const;
		Application*	GetApplication() const;

	protected:
		HDC mHDC;
		HWND mHWND;

	private:
		WMEventHandler* mEventHandler;
		Timer*			mTimer;
		Input*			mInput;
		Renderer*		mRenderer;
		Application*	mApplication;
		bool			mShouldQuit;

		int InitializeMainWindow(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd, Options options);
	};
}