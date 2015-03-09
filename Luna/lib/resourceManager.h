//
// resourceManager.h
//

#ifndef LUNA_RESOURCE_MANAGER_H_INCLUDED
#define LUNA_RESOURCE_MANAGER_H_INCLUDED

#include "lib/type.h"

namespace luna{
	class ResourceManager : public Object, public Singleton<ResourceManager>{
		LUNA_DECLARE_CONCRETE(ResourceManager, Object);

	public:
		class DetouredResource{
		public:
			DetouredResource(class ResourceObject* p) : pObject(p){}
			template<typename T>
			operator T(){ return static_cast<T>(pObject); }
		private:
			class ResourceObject* pObject;
		};

		template<typename T>
		T* load(const c8* name)
		{
			c16 tmpName[MAX_PATH];
			swprintf(tmpName, L"%hs", name);
			return static_cast<T*>(load(tmpName));
		}
		template<typename T>
		T* load(const c16* name)
		{
			return static_cast<T*>(load(name));
		}

	public:
		ResourceManager();
		virtual ~ResourceManager();

		void initialize();
		void finalize();
		bool postUpdate();

		DetouredResource load(const c16* name);
		void reload();

		void deleteNotification(class ResourceObject* objectPtr);

	public:
		void writePackFile(const c16* path);
		void readPackFile(const c16* path, function<void (f32)> cbProgress);
	private:
		void reload(const luna::TypeInfo &dti);
		void reload(ResourceObject* objectPtr);

		bool isReloadRequired(const luna::TypeInfo &dti);
		bool isReloadRequired(ResourceObject* objectPtr);

		bool isDirectoryChanged() const
		{
			return mDirectoryChanged;
		}

		void buildTypeTbl(const luna::TypeInfo& ti);
		ResourceObject* queryCache(const c16* name);
		void enterCache(ResourceObject* objectPtr);
		ResourceObject* loadNew(const c16* name);
		ResourceObject* loadNew(Stream&);

		static DWORD WINAPI directoryWatchProc(void* paramPtr);

	private:
		vector<const luna::TypeInfo*> mResourceTypeTbl;
		vector<class ResourceObject*> mResourceTbl;
		wstring mCurrentDir;
		bool mTerminating;

		bool mDirectoryChanged;
		Mutex mDirectorySync;
		HANDLE mDirectoryWatchThread;
	};
}

#endif /* LUNA_RESOURCE_MANAGER_H_INCLUDED */