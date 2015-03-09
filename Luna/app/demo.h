//
// demo program.
//

#ifndef LUNA_DEMO_H_INCLUDED
#define LUNA_DEMO_H_INCLUDED

#include "app/renderer.h"
#include "app/textureResource.h"
#include "lib/instance.h"
#include "lib/window.h"

namespace luna{
	//! @brief デモアプリケーション
	class Demo : public Instance
	{
		LUNA_DECLARE_CONCRETE(Demo, Instance);

	public:
		Demo();

		virtual void initialize();

		virtual bool run();

		virtual void finalize();

	private:
		struct ExitFade
		{
			XMFLOAT4 color;
		};
		struct Loader
		{
			XMFLOAT4 progress;
			f32 aspectV;
			f32 aspectH;
			f32 padding_;
			f32 padding__;
		};

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static WNDPROC mWndProc;

	private:
		Window::WindowParam mWindowParam;
		unique_ptr<class Window> mWindowPtr;
		unique_ptr<class GraphicsDevice> mGraphicsDevicePtr;
		unique_ptr<class Renderer> mRendererPtr;

		bool mIsPressedEsc;
		ExitFade mExitFade;
		ComPtr<ID3D11Buffer> mExitFadeCB;

		Loader mLoader;
		ComPtr<ID3D11Buffer> mLoaderCB;
		TextureResource* mLoaderTexturePtr;

		ResourceLua* mResourceDemoPtr;
		ResourceLua* mResourceShaderLibPtr;
		ResourceLua* mResourceSceneLibPtr;
		vector<ResourceLua*> mResourceScenePtrTbl;

	private:
		static const DWORD SeekStepTime = 2000;
		static const DWORD SeekBackOffset = 1000;

		bool isPressedEsc() const
		{
			return mIsPressedEsc;
		}
		void setPressedEsc(bool isPressed)
		{
			mIsPressedEsc = isPressed;
		}
		void doLoadScene(f32 progress);
		bool doExitFade();

		void doReboot(bool preservePosition);

		// ウィンドウ管理スレッド化
		bool isExitMessageLoop();
		static DWORD WINAPI threadProcWindow(void*);
		bool createWindow();
		void doMessageLoop();
		s32 getExitCode() const{ return mExitCode; }
		void exitMessageLoop();
		void dispatchMessage();
		void pushMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		enum eMouseButton{
			MB_L = 0x00,
			MB_R,
			MB_M,
		};
		void evMouseUp(s32, s32, s32);
		void evMouseDown(s32, s32, s32);
		void evMouseMove(s32, s32);
		void evMouseWheel(s32);
		void evKeyUp(u32);

		void sceneControlJump(f32 whereMSec);
		void sceneControlRewind(f32 rate);
		void sceneControlFastForward(f32 rate);

		// ウィンドウ管理スレッド化
		HANDLE mhCreateWindow;
		HANDLE mhCreateWindowDone;
		HANDLE mhMessageLoop;
		HANDLE mhMessageLoopDone;
		HANDLE mhWindowThread;
		DWORD mWindowThreadId;
		s32 mExitCode;

		class message{
		public:
			message(HWND hwnd, UINT m, WPARAM wp, LPARAM lp) : hWnd(hwnd), msg(m), wParam(wp), lParam(lp){}

			HWND hWnd;
			UINT msg;
			WPARAM wParam;
			LPARAM lParam;
		};
		std::vector< message > mMessage[2];
		u32 mMessageBufferWrite;
	};
}

#endif // LUNA_DEMO_H_INCLUDED