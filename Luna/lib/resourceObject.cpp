//
// あらゆるデータはResourceObjectを継承したクラスによって
// ロードされ、その情報はResourceManagerによって管理されます。
//
// リソースのロードはResource::load()によって行います。
// 
// 重複したファイルロードを避けるため、
// インスタンスはアプリケーション内部で共有されます。
// そのため、コンストラクタとデストラクタは、
// ファイルがメディアから読み出される時のみ呼び出されます。
//
// リソースクラスは変化する状態を持ってはなりません。
// たとえば、Taskクラスの参照を保持するべきではありません。
// 
// リソースはすべてがリロード可能なつくりになっている必要があります
// TextureやVB/IBのようなデバイスに紐付いたデータをもつ場合は
// リロード時に適切に再作成を行う必要があります。
//

#include "stdafx.h"
#include "resourceObject.h"
#include "resourceManager.h"

namespace luna
{
	LUNA_IMPLEMENT_ABSTRACT(luna::ResourceObject);

	ResourceObject::ResourceObject()
		: mRefPtr(0)
		, mPath()
		, mTime()
		, mIsReloaded(false)
	{
	}

	ResourceObject::~ResourceObject()
	{
	}

	void ResourceObject::addRef()
	{
		++mRefPtr;
	}

	void ResourceObject::release()
	{
		LUNA_ASSERT(mRefPtr, "reference pointer underflow!");
		--mRefPtr;
		if (!mRefPtr){
			ResourceManager::instance().deleteNotification(this);
			delete this;
		}
	}

	s32 ResourceObject::getRef()
	{
		return mRefPtr;
	}

	const wstring& ResourceObject::getPath() const
	{
		return mPath;
	}

	const wstring& ResourceObject::getDir() const
	{
		return mDir;
	}

	const wstring& ResourceObject::getExtension() const
	{
		return mExtension;
	}

	const wstring& ResourceObject::getName() const
	{
		return mName;
	}

	bool ResourceObject::doLoad(Stream& sIn)
	{
		freeResource();

		mPath = sIn.getPath();
		mDir = sIn.getDir();
		mExtension = sIn.getExtension();

		// ファイル名
		c16 szPath[1024];
		wcscpy(szPath, mPath.c_str());
		PathStripPath(szPath);
		PathRemoveExtension(szPath);
		mName = szPath;

#if !LUNA_PUBLISH
		// 更新日付
		HANDLE hFile = CreateFile(sIn.getPath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE){
			GetFileTime(hFile, NULL, NULL, &mTime);
			CloseHandle(hFile);
		}
#endif

		mSubFileTbl.clear();
		File::beginRecording();

		const bool isWentOk = load(sIn);

		File::endRecording(mSubFileTbl);

		return isWentOk;
	}

	bool ResourceObject::doReload()
	{
#if !LUNA_PUBLISH
		File fIn;
		if (!fIn.open(getPath().c_str(), File::OpenMode_Read)){
			return false;
		}
		FileStream sIn(&fIn);
		if (!sIn.isReadable()){
			return false;
		}
		if (!sIn.getSize()){
			return false;
		}

		LUNA_TRACELINE(L"reloading: [%24ls][%12hs].", getName().c_str(), getTypeInfo().getTypeName());
		if (!doLoad(sIn)){
			LUNA_ASSERT(0, L"reload failed. you should restart applicaption.");
			return false;
		}
		LUNA_TRACELINE(L"reloaded : [%24ls][%12hs].", getName().c_str(), getTypeInfo().getTypeName());

		setReloaded(true);
#endif
		return true;
	}
}
