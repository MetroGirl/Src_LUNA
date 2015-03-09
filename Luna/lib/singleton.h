//
// singleton class utilities.
//

#ifndef LUNA_SINGLETON_H_INCLUDED
#define LUNA_SINGLETON_H_INCLUDED

namespace luna{
	template<typename T>
	class Singleton
	{
	public:
		static T& instance()
		{
			if (!mInstancePtr){
				mInstancePtr = new T();
			}
			return *mInstancePtr;
		}

	protected:
		static T* mInstancePtr;
	};

	template<typename T>
	class NonConstructableSingleton
	{
	public:
		static T& instance()
		{
			return *mInstancePtr;
		}

	protected:
		static T* mInstancePtr;
	};
}

#define LUNA_IMPLEMENT_SINGLETON(type_)\
	type_* type_::mInstancePtr = nullptr;

#endif // LUNA_SINGLETON_H_INCLUDED
