//
// window.
//

#ifndef LUNA_WINDOW_H_INCLUDED
#define LUNA_WINDOW_H_INCLUDED

namespace luna{
	//! @brief ひとつのウィンドウを表します
	class Window : public Object{
		LUNA_DECLARE_CONCRETE(Window, Object);

	public:
		class WindowParam{
		public:
			WindowParam(){}
			WindowParam(s32 width, s32 height, const wstring& title, bool fullscreen);

			s32 mWidth;
			s32 mHeight;
			wstring mTitle;
			bool mFullScreen;
		};

		class WindowContext{
		public:
			WindowContext()
				: handle(nullptr), device(nullptr), destroyed(false)
			{
			}

#if LUNA_WINDOWS
			HWND handle;
			HDC device;
#else
			void* handle;
			void* device;
#endif
			int width;
			int height;

			bool destroyed;
		};

	public:
		virtual ~Window();

		static Window* create(const WindowParam& param);
		void destroy();

		const WindowContext& getContext() const
		{
			return mContext;
		}

	private:
		Window();
		void initialize(const WindowParam& param);

#if LUNA_WINDOWS
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

	private:
		WindowParam mParam;
		WindowContext mContext;
	};
}

#endif // LUNA_WINDOW_H_INCLUDED
