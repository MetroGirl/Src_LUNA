#include "stdafx.h"
#include "window.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::Window);

	Window::WindowParam::WindowParam(s32 width, s32 height, const wstring& title, bool fullscreen)
		: mWidth(width), mHeight(height), mTitle(title), mFullScreen(fullscreen)
	{
	}

	Window* Window::create(const Window::WindowParam& param)
	{
		auto* windowPtr = Window::TypeInfo.createInstance<Window>();
		windowPtr->initialize(param);

		return windowPtr;
	}

	Window::Window()
		: mParam(1280, 720, L"", false)
	{
	}

	Window::~Window()
	{
		destroy();
	}

	void Window::initialize(const WindowParam& param)
	{
		mParam = param;

#if LUNA_WINDOWS
		WNDCLASS windowClass;
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

		HMODULE hInstance = GetModuleHandle(NULL);

		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = (WNDPROC)WndProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = hInstance;
		windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = L"demoFramework";

		if (!RegisterClass(&windowClass)) {
			return ;
		}

		RECT rc = { 0, 0, mParam.mWidth, mParam.mHeight };
		AdjustWindowRectEx(&rc, /*WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME*/WS_POPUP, FALSE, dwExStyle);
		mContext.handle = CreateWindowEx(dwExStyle, L"demoFramework", mParam.mTitle.c_str(), /*WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME*/WS_POPUP, CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

		RECT deskrc;
		GetWindowRect(GetDesktopWindow(), reinterpret_cast<LPRECT>(&deskrc));
		GetWindowRect(mContext.handle, reinterpret_cast<LPRECT>(&rc));
		LONG x = (deskrc.right - (rc.right - rc.left)) / 2;
		LONG y = (deskrc.bottom - (rc.bottom - rc.top)) / 2;
		SetWindowPos(mContext.handle, HWND_TOP, x, y, mParam.mWidth, mParam.mHeight, SWP_HIDEWINDOW);


		ShowWindow(mContext.handle, SW_SHOW);
		UpdateWindow(mContext.handle);

		SetProp(mContext.handle, L"thisPtr", (HANDLE)this);

		if (param.mFullScreen){
			while (ShowCursor(FALSE) >= 0);
		}

		mContext.width = param.mWidth;
		mContext.height = param.mHeight;
		mContext.device = GetDC(mContext.handle);
#endif
	}

	void Window::destroy()
	{
		if (mContext.destroyed){
			return;
		}

#if LUNA_WINDOWS
		ReleaseDC(mContext.handle, mContext.device);
		DestroyWindow(mContext.handle);
#endif
		mContext.destroyed = true;
	}

#if LUNA_WINDOWS
	LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* thisPtr = (Window*)GetProp(hWnd, L"thisPtr");

		switch (message) {
			case WM_SIZE:
			{
				if (thisPtr){
					thisPtr->mContext.width = LOWORD(lParam);
					thisPtr->mContext.height = HIWORD(lParam);
				}
			}
			break;
			case WM_DESTROY:
			{
				if (thisPtr){
					thisPtr->destroy();
				}
			}
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
#endif
}
