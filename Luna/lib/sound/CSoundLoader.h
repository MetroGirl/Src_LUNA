/*
	CSoundLoader
*/

#ifndef CSOUNDLOADER_H
#define CSOUNDLOADER_H

#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>
//#define INITGUID
#include <dsound.h>

#define CSL_LOAD_MEMORYIMAGE "ON_MEMORY_FILE_LOAD"

#ifdef _DEBUG
	#ifndef _ASSERT
	#define _ASSERT(exp) \
		do{ \
			if(!exp){ \
				char str[1024]; \
				wsprintf(str, "assertion failed(exp == NULL).\nSource File:%s\nLine:%d", __FILE__, __LINE__); \
				::MessageBox(NULL,str,"ASSERTION FAILED!",MB_SYSTEMMODAL|MB_TOPMOST|MB_ICONSTOP); \
			} \
		}while(0)
	#endif
#else
	#ifndef _ASSERT
	#define _ASSERT
	#endif
#endif

#ifndef QWORD
#define QWORD __int64
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) {if(a!=NULL){(a)->Release(); a=NULL;}}
#endif

#ifndef SAFE_GLOBALFREE
#define SAFE_GLOBALFREE(a) {if(a){GlobalFree(a); a=NULL;}}
#endif

#ifndef SAFE_CLOSEHANDLE
#define SAFE_CLOSEHANDLE(a) {if(a){CloseHandle(a); a=NULL;}}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) {if(a){delete a; a=NULL;}}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) {if(a){delete[] a; a=NULL;}}
#endif

#define CSL_CleanUp(t_ptr) {t_ptr->QueryFree();}

#define CSL_CONTINUE_CURSOR (-1)

typedef struct _tagCSoundLoaderFileInfo{
	int cbsize; //sizeof this structure
	//char path[MAX_PATH]; //
	char name[MAX_PATH]; //
	__int64 fsize;  //size of the physical file.

	void* pMemBuffer;
	__int64 dwMemoryCursor;
}CSL_FILEINFO;

enum {CSL_E_NOTIMPL, CSL_E_OK, CSL_E_UNEXP, CSL_E_NOTLOADED, CSL_E_NOTFOUND, CSL_E_NOMEM, CSL_E_BADFMT, CSL_N_FIN, CSL_E_OUTOFRANGE};

class CSoundLoader{
public:
	CSoundLoader();
	virtual ~CSoundLoader(){}

protected:
	WAVEFORMATEX m_wfx;
	DWORD m_dwCurrentDecodedPos;
	DWORD m_dwDataLength;
	CSL_FILEINFO* m_FileInfo;
	void* m_pData;
	bool isLoaded;
	CRITICAL_SECTION m_csBufferAccess;

public:
	virtual int		GetWaveFormatEx(WAVEFORMATEX* pWfx);
	virtual int		GetDecodedData(
						void** pData,	//pointer to buffer that retrive data. this pointer/buffer must be NULL/empty when first call. If not, library will call GlobalFree();
						long SizeFrom,	//put a minus value to achive sequencial access. otherwise, random.
						long SizeToRead,	//put 0 to read all of data. otherwise, partial.
						bool isLoopWave = TRUE
					);
	virtual int		GetFileInfo(CSL_FILEINFO* pFileInfo);
	virtual DWORD	GetDecodedLengthMs();
	virtual DWORD	GetCurrentDecodedPos();
	virtual int		QueryLoadFile(const char* szFileName, void* pMemData=NULL, DWORD dwMembufferSize = 0);
	virtual int		QueryFree();
	virtual DWORD	GetDecodedLength();
	virtual BOOL	IsLoadable(const char* szFileName, void* pMemData=NULL, DWORD dwMembufferSize = 0);
	virtual int		SetWavePointerPos(DWORD dwPos_byte, int method=SEEK_SET) = 0;
	virtual int		SetWavePointerMS(DWORD dwMS, int method=SEEK_SET) = 0;
protected:
	virtual int ReadWaveData(WAVEFORMATEX* wfx = NULL, void** pdata = NULL, QWORD dwFrom = 0, QWORD dwSizeToRead = 0, bool isLoopWave = FALSE) = 0;
};

#endif

//
//WaveLoader
//
#ifndef WAVELOADER_H
#define WAVELOADER_H

//#include <mmsystem.h>

#define SAFE_MMIOCLOSE(a) { mmioClose(a, 0); a=NULL;}

class WaveLoader:public CSoundLoader{
protected:
	HMMIO m_hmmio;
	DWORD m_dwOffsetToWaveData;
protected:
	int ReadWaveData(WAVEFORMATEX* wfx = NULL, void** pdata = NULL, QWORD dwFrom = 0, QWORD dwSizeToRead = 0, bool isLoopWave = FALSE);
public:
	WaveLoader();
	virtual ~WaveLoader();

public:
	int		QueryLoadFile(const char* szFileName, void* pMemData, DWORD dwMembufferSize);
	int		QueryFree();
	int		SetWavePointerPos(DWORD dwPos_byte, int method);
	int		SetWavePointerMS(DWORD dwMS, int method);
};
#endif

//
//OggVorbisLoader
//
#ifndef OGGVORBISLOADER_H
#define OGGVORBISLOADER_H

//#ifdef __LIB_RELEASE
//typedef struct OggVorbis_File {
//  void            *datasource; /* Pointer to a FILE *, etc. */
//  int              seekable;
//  ogg_int64_t      offset;
//  ogg_int64_t      end;
//  ogg_sync_state   oy;
//
//  /* If the FILE handle isn't seekable (eg, a pipe), only the current
//     stream appears */
//  int              links;
//  ogg_int64_t     *offsets;
//  ogg_int64_t     *dataoffsets;
//  long            *serialnos;
//  ogg_int64_t     *pcmlengths; /* overloaded to maintain binary
//				  compatability; x2 size, stores both
//				  beginning and end values */
//  vorbis_info     *vi;
//  vorbis_comment  *vc;
//
//  /* Decoding working state local storage */
//  ogg_int64_t      pcm_offset;
//  int              ready_state;
//  long             current_serialno;
//  int              current_link;
//
//  double           bittrack;
//  double           samptrack;
//
//  ogg_stream_state os; /* take physical pages, weld into a logical
//                          stream of packets */
//  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
//  vorbis_block     vb; /* local working space for packet->PCM decode */
//
//  ov_callbacks callbacks;
//
//} OggVorbis_File;
//
//typedef struct vorbis_info{
//  int version;
//  int channels;
//  long rate;
//
//  /* The below bitrate declarations are *hints*.
//     Combinations of the three values carry the following implications:
//
//     all three set to the same value:
//       implies a fixed rate bitstream
//     only nominal set:
//       implies a VBR stream that averages the nominal bitrate.  No hard
//       upper/lower limit
//     upper and or lower set:
//       implies a VBR bitstream that obeys the bitrate limits. nominal
//       may also be set to give a nominal rate.
//     none set:
//       the coder does not care to speculate.
//  */
//
//  long bitrate_upper;
//  long bitrate_nominal;
//  long bitrate_lower;
//  long bitrate_window;
//
//  void *codec_setup;
//} vorbis_info;
//
//extern int ov_clear(OggVorbis_File *vf);
//#else

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
//* #endif */

#define SAFE_FPCLOSE(fp) {if(fp){fclose(fp); fp=NULL;}}
#define SAFE_OVCLEAR(ov) {if(ov.vi) {ov_clear(&ov); ZeroMemory(&ov, sizeof(OggVorbis_File));}}

class OggVorbisLoader:public CSoundLoader{
protected:
	OggVorbis_File m_vf;
	FILE* m_fp;
	vorbis_info* m_vi;
protected:
	int DecodeOggVorbis(void** pdata, DWORD dwPlusPointer, DWORD dwSizeToRead, int nQualifyBytes);
	int ReadWaveData(WAVEFORMATEX* wfx = NULL, void** pdata = NULL, QWORD dwFrom = 0, QWORD dwSizeToRead = 0, bool isLoopWave = FALSE);

	static ov_callbacks m_cbOgg;
	static size_t callbackRead(void* ptr, size_t size, size_t nmemb, void* datasource);
	static int callbackSeek(void* datasource, ogg_int64_t offset, int whence);
	static int callbackClose(void* datasource);
	static long callbackTell(void* datasource);

public:
	OggVorbisLoader();
	virtual ~OggVorbisLoader();

public:
	int		QueryLoadFile(const char* szFileName, void* pMemData, DWORD dwMembufferSize);
	int		QueryFree();
	int		SetWavePointerPos(DWORD dwPos_byte, int method);
	int		SetWavePointerMS(DWORD dwMS, int method);
};

#endif