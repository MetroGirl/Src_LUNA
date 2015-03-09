#include "stdafx.h"
#include "CSoundLoader.h"

CSoundLoader::CSoundLoader(){
	m_FileInfo = NULL;
	m_dwCurrentDecodedPos = 0;
	m_dwDataLength = 0;
	m_pData = NULL;
	ZeroMemory(&m_csBufferAccess, sizeof(CRITICAL_SECTION));
}

int CSoundLoader::QueryLoadFile(const char* szFileName, void* pMemData, DWORD dwMembufferSize){
	this->QueryFree();	//make sure all buffers are released.

	//Initialize
	ZeroMemory(&m_wfx, sizeof(WAVEFORMATEX));
	m_dwDataLength = 0;
	m_FileInfo = (CSL_FILEINFO*)GlobalAlloc(GPTR, sizeof(CSL_FILEINFO));
	_ASSERT(m_FileInfo);
	InitializeCriticalSection(&m_csBufferAccess);

	return CSL_E_OK;
}

int CSoundLoader::QueryFree(){
	ZeroMemory(&m_wfx, sizeof(WAVEFORMATEX));
	m_dwDataLength = 0;
	SAFE_GLOBALFREE(m_FileInfo);

	DeleteCriticalSection(&m_csBufferAccess);
	return CSL_E_OK;
}

int	CSoundLoader::GetWaveFormatEx(WAVEFORMATEX* pWfx){
	_ASSERT(pWfx);
	CopyMemory(pWfx, &m_wfx, sizeof(WAVEFORMATEX));
	return CSL_E_OK;
}

int CSoundLoader::GetDecodedData(
					void** pData,	//pointer to buffer that retrive data. this pointer/buffer must be NULL/empty when first call. If not, library will call GlobalFree();
					long SizeFrom,	//put a minus value to achive sequencial access. otherwise, random.
					long SizeToRead,	//put 0 to read all of data. otherwise, partial.
					bool isLoopWave
					)
{
	EnterCriticalSection(&m_csBufferAccess);
		int ret = ReadWaveData(NULL, pData, SizeFrom, SizeToRead, isLoopWave);
	LeaveCriticalSection(&m_csBufferAccess);
	return ret;
}

int	CSoundLoader::GetFileInfo(CSL_FILEINFO* pFileInfo){
	_ASSERT(pFileInfo);
	if(!m_FileInfo){
		return CSL_E_NOTLOADED;
	}

	CopyMemory(pFileInfo, &m_FileInfo, sizeof(CSL_FILEINFO));
	return CSL_E_OK;
}

DWORD CSoundLoader::GetDecodedLengthMs(){
	double dRet = 0;
	dRet = m_dwDataLength / ((double)m_wfx.nAvgBytesPerSec/1000.0);
	return (DWORD)dRet;
}

DWORD CSoundLoader::GetDecodedLength(){
	return m_dwDataLength;
}

BOOL CSoundLoader::IsLoadable(const char* szFileName, void* pMemData, DWORD dwMembufferSize){
	bool isLoadable = (CSL_E_OK == this->QueryLoadFile(szFileName, pMemData, dwMembufferSize));
	this->QueryFree();
	return isLoadable;
}

DWORD CSoundLoader::GetCurrentDecodedPos(){
	return m_dwCurrentDecodedPos;//in byte
}