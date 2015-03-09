//
// runtime type information.
//

#ifndef LUNA_TYPE_H_INCLUDED
#define LUNA_TYPE_H_INCLUDED

#include "cast.h"

namespace luna{
	class Object;
}

namespace luna{
	//! @brief 型情報
	class TypeInfo
	{
	public:
		TypeInfo(const char* typeNameStr, TypeInfo* parentTypePtr);

		//! @brief 型名を取得
		const char* getTypeName(){ return mTypeNameStr; }

		//! @brief 継承元の型情報
		TypeInfo* getParent(){ return mParentPtr; }

		//! @brief 同一継承階層の型情報
		TypeInfo* getNext(){ return mNextPtr; }

		//! @brief 派生先型情報
		TypeInfo* getChild(){ return mChildPtr; }

		//! @brief 継承元の型情報
		const TypeInfo* getParent() const{ return mParentPtr; }

		//! @brief 同一継承階層の型情報
		const TypeInfo* getNext() const{ return mNextPtr; }

		//! @brief 派生先型情報
		const TypeInfo* getChild() const{ return mChildPtr; }

		//! @brief 任意型のインスタンスの作成
		template<typename T>
		T* createInstance() const
		{
			return as<T*>(createInstance());
		}

		//! @brief インスタンスの作成
		virtual Object* createInstance() const = 0;

		//! @brief 抽象型か否か
		virtual bool isAbstract() const = 0;

		//! @brief 指定された型と互換性があるか否か
		bool isA(const TypeInfo& ti) const;

		//! @brief 名前から型情報を検索します
		static TypeInfo* findTypeInfo(const char* nameStr, TypeInfo* tiPtr=nullptr);

		//! @brief 型情報ツリーをトレース出力します
		static void dump(TypeInfo* tiPtr=nullptr);

	private:
		TypeInfo();

		const char* mTypeNameStr;

		TypeInfo* mParentPtr;
		TypeInfo* mNextPtr;
		TypeInfo* mChildPtr;
	};
}

// ルート型であることを指定します
// Objectクラス以外で使わないでください
#define LUNA_DECLARE_ROOT() \
	typedef Object this_t;			\
class ObjectTypeAbstract : public TypeInfo	\
	{								\
	public:							\
	ObjectTypeAbstract(const char* typeNameStr, TypeInfo* parentTypePtr)\
	: TypeInfo(typeNameStr, parentTypePtr)\
		{\
		}\
	\
	virtual bool isAbstract() const { return true; }\
	\
	virtual luna::Object* createInstance() const{\
	return nullptr; \
	}\
	template<typename T>\
	T* createInstance() const\
{\
	return as<T*>(createInstance()); \
}\
};								\
	\
typedef ObjectTypeAbstract typeinfo_t;	\
	virtual void just_for_error_detection() = 0; \
	public:\
	static ObjectTypeAbstract TypeInfo; \
	virtual luna::TypeInfo& getTypeInfo() const{ return this_t::TypeInfo; }\
	private:\
class just_for_typo_error_detection{}

// ルート型であることを指定します
// Objectクラス以外で使わないでください
#define LUNA_IMPLEMENT_ROOT(type_fqn_)\
	::type_fqn_::typeinfo_t type_fqn_::TypeInfo(#type_fqn_, nullptr)

// 
// 抽象型であることを指定します
// クラス宣言のprivateフィールドに配置します
// 
// 抽象型は以下の特徴を持ちます
// ・インスタンスの作成ができない
// 
#define LUNA_DECLARE_ABSTRACT(type_short_name_, parent_type_short_name_)	\
	typedef parent_type_short_name_ base_t;	\
	typedef type_short_name_ this_t;			\
class type_short_name_##TypeAbstract : public TypeInfo	\
{								\
public:							\
	type_short_name_##TypeAbstract(const char* typeNameStr, TypeInfo* parentTypePtr)\
	: TypeInfo(typeNameStr, parentTypePtr)\
	{\
	}\
	\
	virtual bool isAbstract() const { return true; }\
	\
	virtual luna::Object* createInstance() const{\
		return nullptr; \
	}\
	\
	template<typename T>\
	T* createInstance() const\
{\
	return as<T*>(createInstance()); \
}\
};								\
	typedef type_short_name_##TypeAbstract typeinfo_t;	\
	virtual void just_for_error_detection(){}\
	public:\
	static type_short_name_##TypeAbstract TypeInfo; \
	virtual luna::TypeInfo& getTypeInfo() const{ return this_t::TypeInfo; }\
	private:\
	class just_for_typo_error_detection{}

	//
	// 抽象型であることを指定します
	// クラス宣言と対応するソースファイル(.cpp)内に記述します
	//
#define LUNA_IMPLEMENT_ABSTRACT(type_fqn_)\
	::type_fqn_::typeinfo_t type_fqn_::TypeInfo(#type_fqn_, &type_fqn_::base_t::TypeInfo)

	// 
	// 具象型であることを指定します
	// クラス宣言のprivateフィールドに配置します
	// 
	// 具象型は以下の特徴を持ちます
	// ・new演算子によるインスタンスの作成が可能
	// 
#define LUNA_DECLARE_CONCRETE(type_short_name_, parent_type_short_name_)	\
	friend class type_short_name_##TypeConcrete;\
	typedef parent_type_short_name_ base_t;	\
	typedef type_short_name_ this_t;			\
	class type_short_name_##TypeConcrete : public TypeInfo	\
	{								\
	public:							\
		type_short_name_##TypeConcrete(const char* typeNameStr, TypeInfo* parentTypePtr)\
		: TypeInfo(typeNameStr, parentTypePtr)\
		{\
		}\
		\
		virtual bool isAbstract() const { return false; }\
		\
		virtual luna::Object* createInstance() const{\
			return new this_t(); \
		}\
		template<typename T>\
		T* createInstance() const\
		{\
			return as<T*>(createInstance()); \
		}\
	};								\
	typedef type_short_name_##TypeConcrete typeinfo_t; \
	virtual void just_for_error_detection(){ } \
	public:\
	static type_short_name_##TypeConcrete TypeInfo;\
	virtual luna::TypeInfo& getTypeInfo() const{ return this_t::TypeInfo; }\
	private:\
	class just_for_typo_error_detection{}

//
// 具象型であることを指定します
// クラス宣言と対応するソースファイル(.cpp)内に記述します
//
#define LUNA_IMPLEMENT_CONCRETE(type_fqn_)\
	::type_fqn_::typeinfo_t type_fqn_::TypeInfo(#type_fqn_, &type_fqn_::base_t::TypeInfo)

#endif // LUNA_TYPE_H_INCLUDED
