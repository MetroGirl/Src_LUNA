//
// resourceObject.h
//

#ifndef LUNA_RESOURCE_OBJECT_H_INCLUDED
#define LUNA_RESOURCE_OBJECT_H_INCLUDED

#include "lib/type.h"

namespace luna{
	class ResourceObject : public Object
	{
		LUNA_DECLARE_ABSTRACT(ResourceObject, Object);

	public:
		ResourceObject();
		virtual ~ResourceObject();

		void addRef();
		void release();
		s32 getRef();

		const wstring& getPath() const;
		const wstring& getDir() const;
		const wstring& getExtension() const;
		const wstring& getName() const;

		virtual bool load(class Stream&) = 0;
		virtual bool isLoadable(class Stream&) = 0;
		virtual void freeResource() = 0;

		bool doLoad(class Stream&);
		bool doReload();

		const FILETIME& getTime() const { return mTime; }

		bool isReloaded() const{ return mIsReloaded; }
		void setReloaded(bool v){ mIsReloaded = v; }

		bool isValid() const{ return true;  }

		void getSubFiles(vector< pair<FILETIME, wstring> >& vecSubFiles)
		{
			vecSubFiles.insert(vecSubFiles.end(), mSubFileTbl.begin(), mSubFileTbl.end());
		}

	protected:
		s32 mRefPtr;

		wstring mPath;
		wstring mName;
		wstring mExtension;
		wstring mDir;

		FILETIME mTime;

		bool mIsReloaded;

		vector<pair<FILETIME, wstring>> mSubFileTbl;
	};
}

#endif /* LUNA_RESOURCE_OBJECT_H_INCLUDED */