//
// Sound Manager.
//

#ifndef LUNA_SOUND_MANAGER_H_INCLUDED
#define LUNA_SOUND_MANAGER_H_INCLUDED

#include "lib/object.h"

class CSound;
class CSoundLoader;

namespace luna{
	class SoundManager : public Object, public Singleton<SoundManager>{
		LUNA_DECLARE_CONCRETE(SoundManager, Object);

	public:
		SoundManager();
		virtual ~SoundManager();

		bool initialize(HWND hWnd, const char* filename);

		void seek(DWORD posMs);

		void getFFTMagnitude(f32 magnitudeTbl[1024]);

		void play(DWORD posMs=-1);

		void stop();

		bool isPlaying() const;

		u32 getCurPos();

		u32 getDuration() const;

		void setVolume(f32 fVolume);

		f32 getVolume();

		u32 getCurrentTime();

	private:
		DWORD mBassHandle;
		DWORD	mCurTime;

		DWORD	mPrevTime;

		enum eDummyState{
			DS_UNKNOWN,
			DS_PLAY,
			DS_STOP,
		};
		class CSoundLoader* mpLoader;//!< ローダー
		eDummyState mDummyState;
	};
}

#endif // LUNA_SOUND_MANAGER_H_INCLUDED
