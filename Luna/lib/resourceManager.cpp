#include "stdafx.h"
#include "resourceManager.h"
#include "resourceObject.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::ResourceManager);
	LUNA_IMPLEMENT_SINGLETON(ResourceManager);

	ResourceManager::ResourceManager()
		: mResourceTbl()
		, mTerminating(false)
		, mDirectoryChanged(false)
	{
	}

	ResourceManager::~ResourceManager()
	{
	}

	void ResourceManager::initialize()
	{
		c16 szTmpBuffer[1024];
		GetCurrentDirectory(sizeof(szTmpBuffer) / sizeof(*szTmpBuffer), szTmpBuffer);
		mCurrentDir = szTmpBuffer;

		buildTypeTbl(ResourceObject::TypeInfo);

#if !LUNA_PUBLISH
		DWORD dwThreadId;
		mDirectoryWatchThread = CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(directoryWatchProc), this, 0, &dwThreadId);
#endif
	}

	void ResourceManager::finalize()
	{
		mTerminating = true;

		// デモの場合はリーク上等なのでチェックしない
		//for (auto& resourcePtr : mResourceTbl){
		//	LUNA_ERRORLINE(L"detected leak of resource[%ls]. refPtr is [%lu].", resourcePtr->getPath().c_str(), resourcePtr->getRef());
		//}

#if !LUNA_PUBLISH
		WaitForSingleObject(mDirectoryWatchThread, INFINITE);
		CloseHandle(mDirectoryWatchThread);
#endif
	}

	ResourceManager::DetouredResource ResourceManager::load(const c16* filename)
	{
		if (auto loadedPtr = queryCache(filename)){
			return loadedPtr;
		}

		if (auto retPtr = loadNew(filename)){
			enterCache(retPtr);
			return retPtr;
		}

		return nullptr;
	}

	ResourceObject* ResourceManager::queryCache(const c16* name)
	{
		for (auto& objectPtr : mResourceTbl){
			if (objectPtr->getPath() == name){
				if (isReloadRequired(objectPtr)){
					reload(objectPtr);
				}

				objectPtr->addRef();
				return static_cast<ResourceObject*>(objectPtr);
			}
		}
		return nullptr;
	}

	void ResourceManager::enterCache(ResourceObject* objectPtr)
	{
		mResourceTbl.push_back(objectPtr);
	}

	ResourceObject* ResourceManager::loadNew(const c16* filename)
	{
		if (!File::exist(filename)){
			LUNA_ERRORLINE(L"file not found [%ls].", filename);
			return nullptr;
		}

		File fIn;
		if (!fIn.open(filename, File::OpenMode_Read)){
			LUNA_ERRORLINE(L"failed to open file [%ls].", filename);
			return nullptr;
		}
		if (!fIn.getSize()){
			LUNA_ERRORLINE(L"[%ls] has no readable area.", filename);
			return nullptr;
		}
		FileStream sIn(&fIn);

		return loadNew(sIn);
	}

	ResourceObject* ResourceManager::loadNew(Stream& sIn)
	{
		ResourceObject* retPtr = nullptr;
		for (auto& tiPtr : mResourceTypeTbl){
			if (tiPtr->isAbstract()){
				continue;
			}

			retPtr = static_cast<ResourceObject*>(tiPtr->createInstance());
			if (!retPtr){
				LUNA_ASSERT(0, L"out of memory.");
				continue;
			}
			retPtr->addRef();

			sIn.seek(0, File::SeekMode_Set);
			if (retPtr->isLoadable(sIn)){
				LUNA_TRACELINE(L"loading  : [%24ls][%12hs].", sIn.getPath().c_str(), retPtr->getTypeInfo().getTypeName());

				sIn.seek(0, File::SeekMode_Set);
				if (!retPtr->doLoad(sIn)){
					LUNA_ERRORLINE(L"[%s] load failed, while loading with [%s].", sIn.getPath().c_str(), retPtr->getTypeInfo().getTypeName());
					retPtr->release();
					retPtr = nullptr;
					continue;
				}
				LUNA_TRACELINE(L"loaded   : [%24ls][%12hs].", sIn.getPath().c_str(), retPtr->getTypeInfo().getTypeName());
				break;
			}
			else{
				retPtr->release();
				retPtr = nullptr;
			}
		}
		if (!retPtr){
			LUNA_ERRORLINE(L"There're no loadable class for [%ls].", sIn.getPath().c_str());
		}
		return retPtr;
	}

	void ResourceManager::deleteNotification(ResourceObject* objectPtr)
	{
		LUNA_ASSERT(objectPtr->getRef() == 0, L"");

		for (vector<ResourceObject*>::iterator it = mResourceTbl.begin(); it != mResourceTbl.end(); ++it){
			if ((*it) == objectPtr){
				LUNA_TRACELINE(L"unloaded : [%24ls].", (*it)->getPath().c_str());
				mResourceTbl.erase(it);
				break;
			}
		}
	}

	void ResourceManager::writePackFile(const c16* path)
	{
		File::writePackFile(path);
	}

	void ResourceManager::readPackFile(const c16* path, function<void (f32)> cbProgress)
	{
		if (File::readPackFile(path)){
			auto& header = File::getPackHeader();
			auto& tocTbl = File::getPackToc();

			const f32 incPerFile = 1 / (f32)tocTbl.size();

			f32 progress = 0.f;
			cbProgress(progress);
			for (auto& toc : tocTbl){
				load(toc.pathStr);
				progress += incPerFile;
				cbProgress(progress);
			}
			cbProgress(1.f);
		}
		else{
			cbProgress(0.f);
			cbProgress(1.f);
		}
	}

	bool ResourceManager::isReloadRequired(const luna::TypeInfo &dti)
	{
#if !LUNA_PUBLISH
		for (vector<ResourceObject*>::iterator it = mResourceTbl.begin(); it != mResourceTbl.end(); ++it){
			ResourceObject* objectPtr = (*it);
			if (objectPtr->isA(dti) && isReloadRequired(objectPtr)){
				return true;
			}
		}
#endif
		return false;
	}

	bool ResourceManager::isReloadRequired(ResourceObject* objectPtr)
	{
#if !LUNA_PUBLISH
		vector< pair<FILETIME, wstring> > vecFiles;
		vecFiles.push_back(pair<FILETIME, wstring>(objectPtr->getTime(), objectPtr->getPath()));
		objectPtr->getSubFiles(vecFiles);

		bool isUpdated = false;
		for (vector< pair<FILETIME, wstring> >::iterator it = vecFiles.begin(); it != vecFiles.end(); ++it){
			FILETIME mem = it->first;
			FILETIME file;
			HANDLE hFile = CreateFile(it->second.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile != INVALID_HANDLE_VALUE){
				GetFileTime(hFile, nullptr, nullptr, &file);
				CloseHandle(hFile);
			}
			if (CompareFileTime(&file, &mem) == 1){
				isUpdated = true;
				break;
			}
		}
		return isUpdated;
#else
		return false;
#endif
	}

	void ResourceManager::reload()
	{
		reload(ResourceObject::TypeInfo);
	}

	void ResourceManager::reload(const luna::TypeInfo &dti)
	{
#if !LUNA_PUBLISH
		vector<ResourceObject*> vecReload;

		for (vector<ResourceObject*>::iterator it = mResourceTbl.begin(); it != mResourceTbl.end(); ++it){
			ResourceObject* objectPtr = (*it);
			if (objectPtr->isA(dti) && isReloadRequired(objectPtr)){
				vecReload.push_back(objectPtr);
				objectPtr->addRef();
			}
		}

		for (vector<ResourceObject*>::iterator it = vecReload.begin(); it != vecReload.end(); ++it){
			ResourceObject* objectPtr = (*it);
			reload(objectPtr);
			objectPtr->release();
		}
#endif
	}

	void ResourceManager::reload(ResourceObject* objectPtr)
	{
		objectPtr->doReload();
	}

	bool ResourceManager::postUpdate()
	{
#if !LUNA_PUBLISH
		for (vector<ResourceObject*>::iterator it = mResourceTbl.begin(); it != mResourceTbl.end(); ++it){
			ResourceObject* objectPtr = (*it);
			if (objectPtr->isReloaded()){
				objectPtr->setReloaded(false);
			}
		}

		{
			ScopedLock lock(mDirectorySync);
			if (isDirectoryChanged() || ((GetAsyncKeyState(VK_F5) & 0x8000) == 0x8000)){
				reload();
				mDirectoryChanged = false;
			}
		}
#endif
		return true;
	}

	void ResourceManager::buildTypeTbl(const luna::TypeInfo& ti)
	{
		const luna::TypeInfo* tiPtr = &ti;
		while (tiPtr){
			if (tiPtr->isA(ResourceObject::TypeInfo)){
				mResourceTypeTbl.push_back(tiPtr);

				if (tiPtr->getChild()){
					buildTypeTbl(*tiPtr->getChild());
				}
			}
			tiPtr = tiPtr->getNext();
		}
	}

	DWORD WINAPI ResourceManager::directoryWatchProc(void* paramPtr)
	{
#if !LUNA_PUBLISH
		auto* instancePtr = static_cast<ResourceManager*>(paramPtr);
		
		const DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;

		HANDLE hWatch = FindFirstChangeNotification(L"./data/", TRUE, filter);
		if (hWatch == INVALID_HANDLE_VALUE) {
			return 1;
		}

		while (!instancePtr->mTerminating){
			const DWORD waitResult = WaitForSingleObject(hWatch, 100);
			if (waitResult == WAIT_TIMEOUT) {
				continue;
			}

			{
				ScopedLock lock(instancePtr->mDirectorySync);
				instancePtr->mDirectoryChanged = true;
			}

			if (!FindNextChangeNotification(hWatch)){
				break;
			}
		}
#endif
		return 0;
	}
}
