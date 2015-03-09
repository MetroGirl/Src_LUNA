pathStr
dirStr
size
#include "stdafx.h"
#include "file.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::File);

	s32 File::mRecordingPt;
	array<vector<pair<FILETIME, wstring>>, 64> File::mRecordingTblTbl;
	vector<wstring> File::mPackRecordingTbl;
	File File::mPackFile;
	File::PackHeader File::mPackHeader;
	vector<File::PackToc> File::mPackToc;

	File::File()
		: mSize(0)
#if LUNA_WINDOWS
		, mFile(INVALID_HANDLE_VALUE)
#endif
		, mPackHandle(nullptr)
		, mPackPt(0)
	{
	}

	File::~File()
	{
		close();
	}

	bool File::open(const c8* fileName, OpenMode mode, OptionFlag flags)
	{
		c16 tmpName[MAX_PATH];
		swprintf(tmpName, L"%hs", fileName);
		return open(tmpName, mode, flags);
	}

	bool File::open(const c16* fileName, OpenMode mode, OptionFlag flags)
	{
		mPath = fileName;
#if LUNA_WINDOWS
		const c16* extPtr = PathFindExtension(mPath.c_str());
		mExtension = extPtr ? ++extPtr : L"";

		c16 tmpDir[MAX_PATH] = {};
		wcscpy(tmpDir, mPath.c_str());
		c16* p = tmpDir + wcslen(tmpDir);
		while (p != tmpDir){
			if (*p == '/' || *p == '\\'){
				*p = L'\0';
				break;
			}
			--p;
		}
		mDir = tmpDir;
#else
#error implement for yourself.
#endif

		if ((flags&OptionFlag_ByPassPack) == 0){
			if (mPackToc.size()){
				for (auto& toc : mPackToc){
					if (wcscmp(fileName, toc.pathStr) == 0){
						mPackHandle = &toc;
						mPackPt = 0;

						mDir = mPackHandle->dirStr;
						mPath = mPackHandle->pathStr;
						mExtension = mPackHandle->extStr;
						mSize = mPackHandle->size;
						return true;
					}
				}
#if !LUNA_PUBLISH
				LUNA_WARNINGLINE(L"%ls is not packed. opening from native file system...", fileName);
#else
				LUNA_ABORT(L"%ls is not packed.", fileName);
#endif
				// return false;
			}
		}

#if LUNA_WINDOWS
		LUNA_ASSERT(!(flags&OptionFlag_Async), L"unsupported.");

		DWORD dwOpen = 0;
		DWORD dwAccess = 0;
		switch (mode)
		{
		case OpenMode_Read:
			dwOpen = OPEN_EXISTING;
			dwAccess = GENERIC_READ;
			break;
		case OpenMode_ReadWrite:
			dwOpen = (flags & OptionFlag_Truncate) ? OPEN_ALWAYS : CREATE_ALWAYS;
			dwAccess = GENERIC_READ | GENERIC_WRITE;
			break;
		default:
			LUNA_ASSERT(0, L"");
			break;
		}

		mFile = CreateFile(fileName, dwAccess, FILE_SHARE_READ, nullptr, dwOpen, 0, nullptr);
		if (mFile==INVALID_HANDLE_VALUE){
			return false;
		}

		mSize = GetFileSize(mFile, nullptr);

		if ((flags&OptionFlag_NoRecording) == 0){
			if (mRecordingPt != 0){
				const s32 currentPt = mRecordingPt - 1;
				auto it = find_if(mRecordingTblTbl[currentPt].begin(), mRecordingTblTbl[currentPt].end(), [&](const pair<FILETIME, wstring>& rhs)
				{
					return rhs.second == fileName;
				});
				if (it == mRecordingTblTbl[currentPt].end()){
					FILETIME fileTime;
					GetFileTime(mFile, NULL, NULL, &fileTime);
					mRecordingTblTbl[currentPt].push_back(pair<FILETIME, wstring>(fileTime, fileName));
				}
			}

			auto it = find_if(mPackRecordingTbl.begin(), mPackRecordingTbl.end(), [&](const wstring& rhs)
			{
				return rhs == fileName;
			});
			if (it == mPackRecordingTbl.end()){
				mPackRecordingTbl.push_back(fileName);
			}
		}

		return true;
#endif
	}

	void File::close()
	{
#if LUNA_WINDOWS
		if (mFile != INVALID_HANDLE_VALUE){
			CloseHandle(mFile);
			mFile = INVALID_HANDLE_VALUE;
		}
#endif
		mPackHandle = nullptr;
	}

	size_t File::read(void* bufferPtr, size_t sizeToRead)
	{
		if (mPackHandle){
			mPackFile.seek(mPackHandle->offset + mPackPt, File::SeekMode_Set);
			const size_t sizeRead = mPackFile.read(bufferPtr, sizeToRead);
			mPackPt += sizeRead;
			return sizeRead;
		}

#if LUNA_WINDOWS
		DWORD dwSizeRead = 0;
		if (!ReadFile(mFile, bufferPtr, sizeToRead, &dwSizeRead, nullptr)){
			return 0;
		}
		return dwSizeRead;
#else
		return 0;
#endif
	}

	size_t File::write(const void* bufferPtr, size_t sizeToWrite)
	{
		LUNA_ASSERT(mPackHandle == nullptr, L"");

#if LUNA_WINDOWS
		DWORD dwSizeWritten = 0;
		if (!WriteFile(mFile, bufferPtr, sizeToWrite, &dwSizeWritten, nullptr)){
			return 0;
		}
		return dwSizeWritten;
#else
		return 0;
#endif
	}

	size_t File::seek(s64 offset, SeekMode mode)
	{
		if (mPackHandle){
			s64 targetOffset = 0;
			switch (mode)
			{
			case luna::File::SeekMode_Set:
				targetOffset = ((s64)mPackHandle->offset) + offset;
				mode = File::SeekMode_Set;
				mPackPt = (size_t)offset;
				break;
			case luna::File::SeekMode_Cur:
				targetOffset = offset;
				mode = File::SeekMode_Cur;
				mPackPt = (size_t)(((s64)mPackPt) + offset);
				break;
			case luna::File::SeekMode_End:
				targetOffset = ((s64)(mPackHandle->offset + mPackHandle->size)) + offset;
				mode = File::SeekMode_Set;
				mPackPt = (size_t)(((s64)mPackHandle->size) + offset);
				break;
			default:
				break;
			}
			return mPackFile.seek(targetOffset, mode);
		}
#if LUNA_WINDOWS
		DWORD dwMethod = 0;
		switch (mode){
		case SeekMode_Set:
			dwMethod = FILE_BEGIN;
			break;
		case SeekMode_Cur:
			dwMethod = FILE_CURRENT;
			break;
		case SeekMode_End:
			dwMethod = FILE_END;
			break;
		}

		LARGE_INTEGER src, result;
		src.QuadPart = offset;
		if (!SetFilePointerEx(mFile, src, &result, dwMethod)){
			return 0;
		}
		return (size_t)result.QuadPart;
#else
		return 0;
#endif
	}

	bool File::isOpened() const
	{
		if (mPackHandle){
			return true;
		}
#if LUNA_WINDOWS
		return mFile != INVALID_HANDLE_VALUE;
#else
		return false;
#endif
	}

	FILETIME File::getCurrentTime() const
	{
#if LUNA_WINDOWS
		FILETIME fileTime;
		GetFileTime(mFile, NULL, NULL, &fileTime);
		return fileTime;
#else
		return 0;
#endif
	}

	bool File::isNewer(const FILETIME& maybeOld, const FILETIME& maybeNew)
	{
#if LUNA_WINDOWS
		return CompareFileTime(&maybeNew, &maybeOld) == 1;
#else
		return false;
#endif
	}

	bool File::exist(const c16* fileName)
	{
		if (mPackToc.size()){
			for (auto& toc : mPackToc){
				if (wcscmp(fileName, toc.pathStr) == 0){
					return true;
				}
			}
		}

#if LUNA_WINDOWS
		return ::GetFileAttributes(fileName) != 0xffffffff;
#else
		return false;
#endif
	}

	bool File::copy(const c16* srcFileName, const c16* dstFileName, bool isOverWrite)
	{
#if LUNA_WINDOWS
		return CopyFile(srcFileName, dstFileName, isOverWrite ? TRUE : FALSE)!=FALSE;
#else
		return false;
#endif
	}

	bool File::move(const c16* srcFileName, const c16* dstFileName)
	{
#if LUNA_WINDOWS
		return MoveFile(srcFileName, dstFileName)!=FALSE;
#else
		return false;
#endif
	}

	bool File::remove(const c16* fileName)
	{
#if LUNA_WINDOWS
		return DeleteFile(fileName)!=FALSE;
#else
		return false;
#endif
	}

	void File::beginRecording()
	{
		const s32 currentPt = mRecordingPt++;
		LUNA_ASSERT(mRecordingTblTbl[currentPt].empty(), L"");
	}

	void File::endRecording(vector<pair<FILETIME, wstring>>& tbl)
	{
		const s32 currentPt = --mRecordingPt;
		tbl.insert(tbl.end(), mRecordingTblTbl[currentPt].begin(), mRecordingTblTbl[currentPt].end());
		mRecordingTblTbl[currentPt].clear();
	}

	void File::writePackFile(const c16* path)
	{
		File f;
		if (!f.open(path, File::OpenMode_ReadWrite, File::OptionFlag_NoRecording)){
			return;
		}

		PackHeader header;
		header.magic = 0x414b504b;
		header.version = 2;
		header.tocCount = mPackRecordingTbl.size();
		header.reserved = 0;

		vector<PackToc> tocTbl;
		tocTbl.resize(header.tocCount);

		const size_t bodyStart = sizeof(PackHeader) + sizeof(PackToc) * tocTbl.size();
		size_t bodyOffset = bodyStart;
		for (size_t i = 0; i < header.tocCount; ++i){
			auto& toc = tocTbl[i];

			File ifs;
			if (!ifs.open(mPackRecordingTbl[i].c_str(), File::OpenMode_Read, File::OptionFlag_NoRecording)){
				LUNA_ASSERT(0, L"");
			}
			toc.size = ifs.getSize();
			toc.offset = bodyOffset;
			wcscpy(toc.pathStr, ifs.getPath().c_str());
			wcscpy(toc.dirStr, ifs.getDir().c_str());
			wcscpy(toc.extStr, ifs.getExtension().c_str());
			auto fsTime = ifs.getCurrentTime();
			toc.timeLow = fsTime.dwLowDateTime;
			toc.timeHigh = fsTime.dwHighDateTime;

			bodyOffset += toc.size;
		}

		// write
		f.write(&header, sizeof(header));
		f.write(tocTbl.data(), sizeof(tocTbl[0])*tocTbl.size());
		for (size_t i = 0; i < header.tocCount; ++i){
			auto& toc = tocTbl[i];

			File ifs;
			if (!ifs.open(mPackRecordingTbl[i].c_str(), File::OpenMode_Read, File::OptionFlag_NoRecording)){
				LUNA_ASSERT(0, L"");
			}

			LUNA_ASSERT(ifs.getSize() == toc.size, L"");
			u8* bufferPtr = new u8[ifs.getSize()];
			if (ifs.read(bufferPtr, ifs.getSize()) != ifs.getSize()){
				LUNA_ASSERT(0, L"");
			}
			f.write(bufferPtr, ifs.getSize());
			delete[] bufferPtr;
			LUNA_TRACELINE(L"Writing: %ls", toc.pathStr);
		}

		LUNA_TRACELINE(L"Written pack -> %ls", path);
	}

	bool File::readPackFile(const c16* path)
	{
		if (mPackFile.isOpened()){
			return true;
		}
		if (!mPackFile.open(path, File::OpenMode_Read, (OptionFlag)(File::OptionFlag_NoRecording | File::OptionFlag_ByPassPack))){
			return false;
		}

		mPackFile.read(&mPackHeader, sizeof(mPackHeader));
		if (mPackHeader.magic != 0x414b504b || mPackHeader.version != 2){
			LUNA_ERRORLINE(L"Wrong pack magic or version. Disposing pack file...");
			File::remove(path);
			mPackFile.close();
			return false;
		}
		mPackToc.resize(mPackHeader.tocCount);
		mPackFile.read(mPackToc.data(), sizeof(PackToc) * mPackHeader.tocCount);

#if !LUNA_PUBLISH
		for (auto& toc : mPackToc){
			LUNA_TRACELINE(L"TOC: %ls", toc.pathStr);

			File ifs;
			if (ifs.open(toc.pathStr, File::OpenMode_Read, (OptionFlag)(File::OptionFlag_NoRecording | File::OptionFlag_ByPassPack))){
				auto fsTime = ifs.getCurrentTime();

				FILETIME fsMem;
				fsMem.dwLowDateTime = toc.timeLow;
				fsMem.dwHighDateTime = toc.timeHigh;
				if (File::isNewer(fsMem, fsTime)){
					LUNA_ERRORLINE(L"Packed file contains older version. Disposing pack file...");
					mPackFile.close();
					mPackToc.clear();
					File::remove(path);
					return false;
				}
			}
		}
#endif

		LUNA_TRACELINE(L"Opened pack <- %s", path);
		return true;
	}
}