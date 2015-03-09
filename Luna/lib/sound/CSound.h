//###			DirectSound Library 		 ###
//###	Code, Design : c.r.v. 2004-2005		 ###
/*
	version:
		0.1 - blog公開版
		0.5 - box
		0.6 - box add :: GetFrequency(), 'virtual'
		0.7 - 初期化をプログラマの責任に.
		      内部処理分割.
			  デュプリケート対応.
		0.71- 学内でコンパイルできるように.
		      遅いマシンだと,デュプリケートに時間がかかることが発覚.
			  使用メモリ削減も兼ね,そこら辺を改良.
			  メソッド名変更.GetPosition -> GetCurrentPosision
		0.72- Wave読み込み部分を改善.高速に.
		0.73- Wave読み込み高速化に伴うバグ修正.
		0.74- プライマリバッファの管理方式を変更.使用メモリ削減.
		0.75- フールプルーフ山盛り.
		0.76- デストラクタのバグ修正
		0.77- 型変換用のオペレータ.仮実装に過ぎず, 今後変更する可能性もある.
		0.78- メソッドをいろいろ変更. 後始末もプログラマの責任に.一部変数名変更.
		0.79- フールプルーフちょっと増量.
		0.80- ラップアラウンド対応::WAVEファイル読み込み
		0.81- WriteDataToBuffer()の引数を変更.ストリーミング対応化.
		0.82- Stop()を書き換え.
		0.83- 複製バッファ再生時に、常にメインバッファの再生位置が０になる問題を修正
		0.84- サウンドファイルの展開は外部クラスで行うことにしたので、書き換え中。
		0.90- CSoundLoaderとの連携部分完成。
				
	date:
		-0.82 05/31/2005
		0.83- Oct.21.2005

	notice:
		mp3は特許問題のためサポートしない。
		DirectShowのデコーダに投げてやることもできるけど、作品を売ると金取られるので却下。
		どーしてもmp3じゃなきゃ嫌な人は勝手にデコーダに投げて得られたWAVEを渡すかほかのライブラリを使ってください。
*/
#ifndef CSOUND_H_DEFINED
#define CSOUND_H_DEFINED

#include "CSoundLoader.h"
#include <windows.h>

#define CS_LIMITLOADONMEMORY 1024*1024*1	//メモリ上に展開する最大容量(1mB)。これを超えるとストリーミング再生。
#define CS_NOSOUND		-10000
#define CS_VOLUME_DB(csdB__)   ((csdB__)*100)
#define CS_VOLUME_MAX        0
#define CS_VOLUME_MIN	-10000
#define CS_VOLUME_RANGE  10000
#define CS_VOLUME_RANGE_REAL 10000
#define CS_VOLUME_FIX_SET(csVolumeToSet__) (LONG)(csVolumeToSet__/**(CS_VOLUME_RANGE_REAL/CS_VOLUME_RANGE)*/)/*CS_VOLUME_RANGE_REAL-CS_VOLUME_RANGE*/
#define CS_VOLUME_FIX_GET(csVolumeToGet__) (LONG)(csVolumeToGet__/*/(CS_VOLUME_RANGE_REAL/CS_VOLUME_RANGE)*/)
#define CS_PAN_RIGHT	 10000
#define CS_PAN_LEFT		-10000

#define CSL_MSG_SEEK			WM_USER+43+1
#define CSL_MSG_GETCURPOS_BYTE  WM_USER+43+2
#define CSL_MSG_STOP			WM_USER+43+3
#define CSL_MSG_SEEK_AND_PLAY   WM_USER+43+4
#define CSL_MSG_PAUSE			WM_USER+43+5

enum {CS_E_NOTIMPL, CS_E_OK, CS_E_UNEXP, CS_E_NOTLOADED, CS_E_NOTFOUND, CS_E_NULL_OBJECT, CS_E_NULL_PRIMARY, CS_E_NULL_SECONDARY, CS_E_NULL, CS_E_NOMEM, CS_E_NOCANDO};

class CSound
{
protected:
	static HMODULE m_hDLL;

#if DIRECTSOUND_VERSION >= 0x0800
	static IDirectSound8* m_pSoundObject;
#else
	static IDirectSound* m_pSoundObject;
#endif
	static IDirectSoundBuffer*	m_pPrimaryBuffer;
	static int m_nRef;
	IDirectSoundBuffer* m_pSecondaryBuffer;
	volatile bool m_isLoop;

	WAVEFORMATEX m_wfx;

	int m_nDuplicateLimit;
	IDirectSoundBuffer** m_ppDuplicatedBuffer;

	CSoundLoader*		m_Loader;
	
	volatile DWORD m_dwOneSplittedBufferSize;
	volatile DWORD m_dwNotificationNum;
	volatile DWORD m_dwBufferLengthSec;

	BOOL	m_isStreamFile;

	DWORD m_dwThreadId;
	HANDLE m_hThread;

	volatile HANDLE m_pNotifyHandle;
	IDirectSoundNotify* m_pSoundNotify;
	DSBPOSITIONNOTIFY* m_pDsbnotify;

	bool m_isAllowRapidAccess;

	//CRITICAL_SECTION m_csWriteBuffer;

	volatile HANDLE m_hThreadMessageDispatchEvent;

protected:
	static void	MessageBox(const char* format, ...);
	int			GetPrimaryBuffer(IDirectSoundBuffer** buffer, DWORD dwFlags);	//プライマリバッファを取得してくる
	bool		CreateBuffer(IDirectSoundBuffer** buffer, DSBUFFERDESC* dsbdesc, WAVEFORMATEX* wfx);	//バッファ作成.
	bool		WriteDataToBuffer(IDirectSoundBuffer** buffer, void* data, DWORD dwOffset, DWORD dwWriteSize);	//バッファにWAVEデータを転送する.
	int			GetInactiveBufferNo();	//使用されていない複製バッファIDを取り出す
public:
	static int			GetLoaderInterface(CSoundLoader** ppLoader, const char* szFileName, void* pMemData=NULL, DWORD dwMembufferSize = 0);
protected:
	static		DWORD WINAPI StreamThread(LPVOID CSSPtr);
	void		CloseStreamThread();
	//void		OpenStreamThread();
	void		QueryInitialize();
	int			LoadInternal(const char* szFileName, DSBUFFERDESC* pDsbdesc, void* pData = NULL, unsigned long dwDataSize = 0);


	bool IsDuplicatedBufferPlaying();
	int StopDuplicatedBuffer();
	int SetStreamCurosr(DWORD dwResumePosMs);

public:
	CSound();
	CSound(HWND hWnd, DWORD dwCoopLevel = DSSCL_PRIORITY);
	~CSound();
public:
	int AddRef(){ return ++m_nRef; }
	int Release(){ return --m_nRef; }

	int Initialize(HWND hWnd, DWORD dwCoopLevel = DSSCL_PRIORITY);	//初期化
	int UnInitialize();												//破棄

	int Load(const char* szFileName, DSBUFFERDESC* pDsbdesc = NULL);
	//int LoadWaveFromMemory(void* pdata, WAVEFORMATEX* wfx, DWORD dwLength_byte, DSBUFFERDESC* pDsbdesc = NULL);
	int LoadMemoryImage(void* pData, unsigned long dwDataSize);
	int Play(DWORD position_ms = 0, bool isLoop = false);
	int Stop();

	int Reset();
	bool IsPlaying();

	bool IsLoaded(){ return (m_Loader!=NULL); }

	DWORD GetDuration();
	DWORD GetCurrentPosition();	//再生位置をms単位で取得
	DWORD GetFrequency();		//周波数をHz単位で取得
	LONG  GetVolume();
	LONG  GetPan();

	void SetPan(LONG nPan);				//パン値のレンジ -> 左:-10,000 右:10,000
	void SetVolume(LONG nVolume);		//ボリューム値のレンジ(1/100dB) -> 無調整:0 無音:-10,000 増幅はサポートされない
	void SetFrequency(DWORD nFrequency);	//周波数値(Hz) -> 無調整:0
	void SetMasterPanAndVolume(LONG nVol = 0, LONG nPan = 0);

	//プライマリバッファ（出力バッファ）のフォーマットを設定する
	//デフォルト -> 2Channels, 44.1kHz, 16bits
	int SetPrimaryBufferWaveFormat(WORD Channels, DWORD SamplesPerSec, WORD BitsPerSample);

	void DuplicateBuffer(DWORD nDuplicate);	//デュプリケート(FXとストリーミングには使えない)
	void		AllowRapidAccess(bool isAllow);

	void PrepareFFT();
	f32 readMagnitude(u32 msec);
	f32 readdB(u32 msec);
	void grabMagnitude(u32 msec, f32* dest, u32 count);
	void grabdB(u32 msec, f32* dest, u32 count);
	static const u32 OFFSET_CONTINUE = 0;
	void PushFFT(void* pdata, u32 sizeData, u32 offset=OFFSET_CONTINUE);
	void FlushFFT();

	u32 millisecToIndex(u32 msec);
	
	u32 mFFTPt;
	u32 mSpectPt;
	u32 mFFTSize;
	f32* mpMagnitude;
	f32* mpdB;
	f32* mpFFTBase;
	f32* mpFFTTmp;
	
	void PushBuffer(u32 sizeData, u32 offset=OFFSET_CONTINUE);
	void FlushBuffer();
	u32 getCurrentTime();

	u32 mTimeBufferSize;
	u32 mTimeBufferOffset;

	u32 pollSound();

	CRITICAL_SECTION mCsPollSound;
	bool mInternalPlaying;
	DWORD mNextWriteOffset;

	DWORD getInternalNextWriteOffset()
	{
		return mNextWriteOffset;
	}

	void setInternalNextWriteOffset(DWORD v)
	{
		mNextWriteOffset = v;
	}

	void setInternalPlaying(bool v)
	{
		mInternalPlaying = v;
	}
	bool isInternalPlaying()
	{
		return mInternalPlaying;
	}
};
#endif