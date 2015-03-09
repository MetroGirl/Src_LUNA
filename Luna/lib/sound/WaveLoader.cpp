#include "stdafx.h"
#include "./WaveLoader.h"
#pragma warning(disable:4244)

WaveLoader::WaveLoader(){
	m_hmmio = NULL;
	m_dwOffsetToWaveData = 0;
}

WaveLoader::~WaveLoader(){
	this->QueryFree();
}

int WaveLoader::QueryLoadFile(const char* szFileName, void* pMemData, DWORD dwMembufferSize){
	int err = this->CSoundLoader::QueryLoadFile(szFileName);
	if(err!=CSL_E_OK) return err;

	m_dwDataLength = 0;
	m_dwCurrentDecodedPos = 0;

	//Set CSL_FILEINFO
	m_FileInfo->cbsize = sizeof(CSL_FILEINFO);
	strcpy(m_FileInfo->name, szFileName);
	//char* str = (char*)GlobalAlloc(GPTR, strlen(szFileName)+1);
	//_ASSERT(str);
	//	strcpy(str, szFileName);
	//	PathRemoveFileSpec(str);
	//	strcpy(m_FileInfo->path, str);

	//	strcpy(str, szFileName);
	//	PathStripPath(str);
	//	strcpy(m_FileInfo->name, str);
	//  GlobalFree(str); 

	if( strcmp(m_FileInfo->name, CSL_LOAD_MEMORYIMAGE)!=0 ) {
		HFILE hFile = (HFILE)CreateFileA(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		//_ASSERT(hFile);
//#if _MSC_VER >= 1310
//		GetFileSizeEx((HANDLE)hFile, (PLARGE_INTEGER)&m_FileInfo->fsize);
//#else
		m_FileInfo->fsize = GetFileSize((HANDLE)hFile, NULL);
//#endif
		CloseHandle((HANDLE)hFile);
	}else{
		m_FileInfo->fsize = dwMembufferSize;
		m_FileInfo->pMemBuffer = pMemData;
	}
	
	if(CSL_E_OK != ReadWaveData(&m_wfx, NULL)){
		CSL_CleanUp(this);
		return CSL_E_BADFMT;
	}

	this->isLoaded = true;
	return CSL_E_OK;
}

int WaveLoader::QueryFree(){
	//WAVE
	m_dwOffsetToWaveData = 0;
	SAFE_MMIOCLOSE(m_hmmio);
	this->isLoaded = false;

	//SAFE_GLOBALFREE(m_pData);
	return this->CSoundLoader::QueryFree();
}

//WAVEファイルをロードする
int WaveLoader::ReadWaveData(WAVEFORMATEX* wfx, void** pdata, QWORD dwFrom, QWORD dwSizeToRead, bool isLoopWave)
{
	static MMCKINFO parent, child;
	static char szBefore[MAX_PATH];
	_ASSERT(m_FileInfo->name);
	if(pdata!=NULL) SAFE_GLOBALFREE(*pdata);

	//if(m_hmmio!=NULL && strcmp(m_FileInfo->name, szBefore)!=0){//ファイル名が指定されていたら開きなおす
	//	SAFE_MMIOCLOSE(m_hmmio);
	//}
	if( !isLoaded ){
		strcpy(szBefore, m_FileInfo->name);

		if( strcmp(m_FileInfo->name, CSL_LOAD_MEMORYIMAGE)==0 ){
//			if(!m_hmmio){//最初の一回しかしない処理（ストリーミング用の処置）
			MMIOINFO mmioinfo;
			ZeroMemory(&mmioinfo, sizeof(MMIOINFO));
			mmioinfo.pchBuffer = (HPSTR)m_FileInfo->pMemBuffer;
			mmioinfo.fccIOProc = FOURCC_MEM;
			mmioinfo.cchBuffer = m_FileInfo->fsize;

			if(NULL == (m_hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ|MMIO_ALLOCBUF))){
				return CSL_E_UNEXP;
			}
		}else{
			if(NULL == (m_hmmio = mmioOpenA((LPSTR)m_FileInfo->name, NULL, MMIO_READ|MMIO_ALLOCBUF))){
				return CSL_E_UNEXP;
			}
		}
		parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');//waveファイルかどうか調べる
		if(mmioDescend(m_hmmio, &parent, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR){
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}
		child.ckid = mmioFOURCC('f', 'm', 't', ' ');//fmtチャンクへ移動する
		if(mmioDescend(m_hmmio, &child, &parent, MMIO_FINDCHUNK) != MMSYSERR_NOERROR){
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}

		_ASSERT(wfx);
		if(mmioRead(m_hmmio, (HPSTR)wfx, (LONG)child.cksize) != (LONG)child.cksize){//fmtチャンク(WAVEFORMATEX)読み取り
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}
		_ASSERT((wfx->wFormatTag==WAVE_FORMAT_PCM));
		mmioAscend(m_hmmio, &child, 0);//fmtチャンクから出る
		child.ckid = mmioFOURCC('d', 'a', 't', 'a');//dataチャンクに移動
		if(mmioDescend(m_hmmio, &child, &parent, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}
		m_dwDataLength = child.cksize;//WAVE領域のサイズ
		m_dwOffsetToWaveData = mmioSeek(m_hmmio, 0, SEEK_CUR);//データまでの位置を保存しておく
	}
	if(pdata){
		if(dwSizeToRead<=0){//ファイル全体を読み込む
			(*pdata) = (LPBYTE)GlobalAlloc(GPTR, m_dwDataLength * sizeof(BYTE));
			_ASSERT(*pdata);

			if(mmioRead(m_hmmio, (HPSTR)*pdata, (LONG)m_dwDataLength) != (LONG)m_dwDataLength){
				GlobalFree(*pdata);
				SAFE_MMIOCLOSE(m_hmmio);
				return CSL_E_UNEXP;
			}
			SAFE_MMIOCLOSE(m_hmmio);	//必要なデータがそろったので､ファイルを閉じる
			m_dwCurrentDecodedPos += dwSizeToRead;
		}else{
			//DWORD dwInnerFinPos = dwFrom + dwSizeToRead;
			////領域サイズ以上だったら収まるように値を補正
			//if(m_dwDataLength < dwInnerFinPos){
			//	dwSizeToRead -= dwInnerFinPos-m_dwDataLength;
			//}

			//開始位置が指定されていれば, データ領域からのオフセットをStartとする.
			//指定されていなければ, それまで進んだカーソル位置から読み込みを開始する
			if(dwFrom>=0) mmioSeek(m_hmmio, (LONG)(m_dwOffsetToWaveData + dwFrom), SEEK_SET);
			
			//要求領域分のメモリ確保
			(*pdata) = (LPBYTE)GlobalAlloc(GPTR, (SIZE_T)(dwSizeToRead * sizeof(BYTE)));
			_ASSERT(*pdata);

			//現在位置からリクエストサイズを読むとオーバーするようなら、ラップアラウンドする。
			DWORD dwNowCursor = mmioSeek(m_hmmio, 0, SEEK_CUR) - m_dwOffsetToWaveData;
			if(m_dwDataLength < (dwNowCursor + dwSizeToRead)){
				if( !isLoopWave ){
					if( dwNowCursor>=m_dwDataLength ){
						FillMemory((BYTE*)*pdata, dwSizeToRead, (m_wfx.wBitsPerSample==8) ? 128:0);
						return CSL_E_OUTOFRANGE;
					}
				}

				DWORD dwBeforeWrapAround = m_dwDataLength-dwNowCursor; 
				//とりあえず、最後まで読む
				if(mmioRead(m_hmmio, (HPSTR)*pdata, (LONG)dwBeforeWrapAround) != (LONG)dwBeforeWrapAround){
					GlobalFree(*pdata);
					SAFE_MMIOCLOSE(m_hmmio);
					return CSL_E_UNEXP;
				}
				m_dwCurrentDecodedPos += dwBeforeWrapAround;

				if(isLoopWave){//残った部分を無音で埋めるかどうか
					mmioSeek(m_hmmio, 0, SEEK_SET);
					mmioSeek(m_hmmio, m_dwOffsetToWaveData, SEEK_CUR);	//ポインタをWAVE領域始点に戻す
					//ラップアラウンド分を読む
					if(mmioRead(m_hmmio, (HPSTR)*pdata+dwBeforeWrapAround, (LONG)(dwSizeToRead - dwBeforeWrapAround)) != (LONG)(dwSizeToRead - dwBeforeWrapAround)){
						GlobalFree(*pdata);
						SAFE_MMIOCLOSE(m_hmmio);
						return CSL_E_UNEXP;
					}
					m_dwCurrentDecodedPos =  0;
					m_dwCurrentDecodedPos += dwSizeToRead - dwBeforeWrapAround;
				}else{
					FillMemory((BYTE*)*pdata+dwBeforeWrapAround, dwSizeToRead - dwBeforeWrapAround, (m_wfx.wBitsPerSample==8) ? 128:0);
					return CSL_N_FIN;
				}
			}else{ //ラップアラウンドしなかった場合
				if(mmioRead(m_hmmio, (HPSTR)*pdata, (LONG)dwSizeToRead) != (LONG)dwSizeToRead){
					GlobalFree(*pdata);
					SAFE_MMIOCLOSE(m_hmmio);
					return CSL_E_UNEXP;
				}
				m_dwCurrentDecodedPos += dwSizeToRead;
			}//必要なデータはまだあるので、ファイルは閉じない
		}
	}
	return CSL_E_OK;
}

int WaveLoader::SetWavePointerPos(DWORD dwPos_byte, int method)
{
	if( isLoaded ){
	//if(m_hmmio){
		m_dwCurrentDecodedPos = dwPos_byte;
		mmioSeek(m_hmmio, dwPos_byte+m_dwOffsetToWaveData, method);
		return CSL_E_OK;
	//}
	}
	return CSL_E_NOTLOADED;
}

int WaveLoader::SetWavePointerMS(DWORD dwMS, int method){
	if( isLoaded ){
		m_dwCurrentDecodedPos = dwMS/1000.0 * (double)m_wfx.nAvgBytesPerSec;
		mmioSeek(m_hmmio, (dwMS/1000.0 * (double)m_wfx.nAvgBytesPerSec) + m_dwOffsetToWaveData, method);
	}
	return CSL_E_OK;
}