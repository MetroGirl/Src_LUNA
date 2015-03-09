//
// Settings.
//
//

#ifndef LUNA_SETTINGS_MANAGER_H_INCLUDED
#define LUNA_SETTINGS_MANAGER_H_INCLUDED

#include "lib/type.h"
#include "lib/object.h"
#include "lib/singleton.h"

namespace luna{
	class SettingsManager : public Object, public Singleton<SettingsManager>{
		LUNA_DECLARE_CONCRETE(SettingsManager, Object);

	public:
		bool initialize();

		bool isFullScreen() const
		{
			return mRunSettings.isFullscreen;
		}

		s32 getWidth() const
		{
			return mRunSettings.nWidth;
		}

		s32 getHeight() const
		{
			return mRunSettings.nHeight;
		}

		s32 getBpp() const
		{
			return mRunSettings.nBpp;
		}

		s32 getAspectH() const
		{
			return mRunSettings.nAspH;
		}
		s32 getAspectV() const
		{
			return mRunSettings.nAspV;
		}

		bool isVSync() const
		{
			return mRunSettings.isVSync;
		}

		s32 getMSAA() const
		{
			return mRunSettings.nMsaa;
		}
		void setMSAA(s32 msaa)
		{
			msaa = max(msaa, 0);
			mRunSettings.nMsaa = msaa;
		}

		bool isAlwaysOnTop() const
		{
			return mRunSettings.isAlwaysOnTop;
		}

		bool isNoSound() const
		{
			return mRunSettings.isNoSound;
		}

		bool isBigScreenMode() const
		{
			return mRunSettings.isBigScreenMode;
		}

		bool isLowQuality() const
		{
			return mRunSettings.isLowQuality;
		}

		bool isLoopDemo() const
		{
			return mRunSettings.isLoopDemo;
		}
		void setLoopDemo(bool isLoop)
		{
			mRunSettings.isLoopDemo = isLoop;
		}

		s32 getFrequency() const
		{
			return mRunSettings.nFrequency;
		}
		void setFrequency(s32 v)
		{
			mRunSettings.nFrequency = v;
		}


	private:
		static LRESULT CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

	private:
		struct DemoSettings{
			s32 nWidth, nHeight;
			s32 nFrequency;
			s32 nBpp;
			s32 nAspH, nAspV;
			bool isFullscreen;
			bool isVSync;
			s32  nMsaa;
			bool isAlwaysOnTop;
			bool isNoSound;
			bool isBigScreenMode;
			bool isLowQuality;
			bool isLoopDemo;
		};
		struct Resolution{
			Resolution()
				: w(0), h(0), hz(0)
			{
			}
			Resolution(s32 width, s32 height, s32 refresh)
				: w(width), h(height), hz(refresh)
			{
			}
			bool operator<(const Resolution& that) const
			{
				return (w==that.w) ? h<that.h : w<that.w;
			}

			s32 w, h;
			s32 hz;
		};
		struct AspectRatio{
			f32 h, v;
		};

	private:
		static DemoSettings defaultSettings;

		vector<Resolution> mVecResolution;
		DemoSettings mRunSettings;
	};
}

#endif // LUNA_SETTINGS_MANAGER_H_INCLUDED
