//
// basic object type.
//

#ifndef LUNA_OBJECT_H_INCLUDED
#define LUNA_OBJECT_H_INCLUDED

#include "lib/type.h"

namespace luna{
	//! @brief 基底オブジェクト
	//! 
	//! 型情報に基づいたサービスを享受するためには、
	//! このクラスを継承する必要があります。
	class Object
	{
		LUNA_DECLARE_ROOT();

	public:
		//! @brief このクラスが指定された型と互換性があるか否か
		//! 互換性のある型に対してはキャストが可能
		template<typename T>
		bool isA() const
		{
			return getTypeInfo().isA(T::TypeInfo);
		}

		//! @brief このクラスが指定された型と互換性があるか否か
		//! 互換性のある型に対してはキャストが可能
		bool isA(const luna::TypeInfo& ti) const
		{
			return getTypeInfo().isA(ti);
		}
	};
}

#endif // LUNA_OBJECT_H_INCLUDED
