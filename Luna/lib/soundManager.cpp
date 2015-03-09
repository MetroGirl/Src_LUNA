#include "stdafx.h"
#include "soundManager.h"

#include "lib/sound/CSound.h"
#include "lib/sound/CSoundLoader.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::SoundManager);
	LUNA_IMPLEMENT_SINGLETON(luna::SoundManager);


	SoundManager::SoundManager()
		: mBassHandle(0), mCurTime(0), mPrevTime(0), mpLoader(nullptr), mDummyState(DS_STOP)
	{
		if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
			LUNA_ASSERT(0, L"");
		}
		if (!BASS_Init(-1, 44100, 0, 0, NULL)){
			LUNA_ASSERT(0, L"");
		}
	}

	SoundManager::~SoundManager()
	{
		BASS_Free();
	}

	bool SoundManager::initialize(HWND hWnd, const char* filename)
	{
		BASS_MusicFree(mBassHandle);
		BASS_StreamFree(mBassHandle);

		if (!(mBassHandle = BASS_StreamCreateFile(FALSE, filename, 0, 0, 0))
			&& !(mBassHandle = BASS_MusicLoad(FALSE, filename, 0, 0, BASS_MUSIC_RAMP, 1)))
		{
			return false;
		}

		if (CS_E_OK != CSound::GetLoaderInterface(&mpLoader, filename, NULL, 0)){
			return false;
		}
		if (CSL_E_OK != mpLoader->QueryLoadFile(filename, NULL, 0)){
			return false;
		}

		return true;
	}

	void SoundManager::seek(DWORD posMs)
	{
		LUNA_ASSERT(!isPlaying(), L"");
		mCurTime = posMs;
	}

	void SoundManager::getFFTMagnitude(f32 magnitudeTbl[1024])
	{
		if (mBassHandle){
			BASS_ChannelGetData(mBassHandle, magnitudeTbl, BASS_DATA_FFT2048);
		}
	}

	//! @brief 再生
	void SoundManager::play(DWORD posMs)
	{
		if (posMs != -1){
			const bool isOutOfRange = posMs >= getDuration();
			if (isOutOfRange){
				posMs = getDuration();
				stop();
				return;
			}
		}

		if (mBassHandle){
			BASS_ChannelPlay(mBassHandle, FALSE);
			if (posMs != -1){
				QWORD posByte = BASS_ChannelSeconds2Bytes(mBassHandle, posMs / 1000.0);
				BASS_ChannelSetPosition(mBassHandle, posByte, BASS_POS_BYTE);
			}
		}
		else{
			mDummyState = DS_PLAY;
		}

		mCurTime = posMs;
		mPrevTime = timeGetTime();
	}

	//! @brief 停止
	void SoundManager::stop()
	{
		if (mBassHandle){
			BASS_ChannelStop(mBassHandle);
		}
		else{
			mDummyState = DS_STOP;
		}
	}

	//! @brief 再生中か否か
	bool SoundManager::isPlaying() const
	{
		if (mBassHandle){
			return BASS_ChannelIsActive(mBassHandle) == BASS_ACTIVE_PLAYING;
		}
		return mDummyState == DS_PLAY;
	}

	//! @brief 再生中のカーソル位置を取得
	u32 SoundManager::getCurPos()
	{
		DWORD curSysTime = timeGetTime();

		if (isPlaying()){
			mCurTime += (curSysTime - mPrevTime);
			//return mpSound->GetCurrentPosition();
		}

		if (mpLoader){// ダミーシステムの場合はステート切り替え
			if (mCurTime >= getDuration()){
				mDummyState = DS_STOP;
			}
		}

		mPrevTime = curSysTime;

		return mCurTime;
	}

	//! @brief ソースの長さを取得
	u32 SoundManager::getDuration() const
	{
		if (!mpLoader){
			return 0;
		}
		return mpLoader->GetDecodedLengthMs();
	}

	//! @brief ボリュームを設定(1.fが最高)
	void SoundManager::setVolume(f32 fVolume)
	{
		if (mBassHandle){
			BASS_ChannelSetAttribute(mBassHandle, BASS_ATTRIB_VOL, fVolume);
		}
	}

	//! @brief ボリュームを取得
	f32 SoundManager::getVolume()
	{
		if (mBassHandle){
			f32 value = 0.f;
			BASS_ChannelGetAttribute(mBassHandle, BASS_ATTRIB_VOL, &value);
			return value;
		}
		return 0.f;
	}

	//! @brief 現在の再生位置をミリ秒で取得
	u32 SoundManager::getCurrentTime()
	{
		if (mBassHandle){
			const QWORD pos = BASS_ChannelGetPosition(mBassHandle, BASS_POS_BYTE);
			const double pos_time = BASS_ChannelBytes2Seconds(mBassHandle, pos);
			return static_cast<u32>(pos_time * 1000.0f);
		}
		return getCurPos();
	}
}
