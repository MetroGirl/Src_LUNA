#include "stdafx.h"
#include "type.h"

namespace luna{
	TypeInfo::TypeInfo(const char* typeNameStr, TypeInfo* parentTypePtr)
		: mTypeNameStr(typeNameStr), mParentPtr(parentTypePtr)/*, mNextPtr(nullptr)*/
	{
		if (mParentPtr){
			TypeInfo** writeDestPtrPtr = &mParentPtr->mChildPtr;
			while (*writeDestPtrPtr){
				writeDestPtrPtr = &(*writeDestPtrPtr)->mNextPtr;
			}
			*writeDestPtrPtr = this;
		}
	}

	bool TypeInfo::isA(const TypeInfo& ti) const
	{
		const TypeInfo* tiPtr = this;

		while (tiPtr){
			if (tiPtr == &ti){
				return true;
			}

			tiPtr = tiPtr->getParent();
		}
		return false;
	}

	TypeInfo* TypeInfo::findTypeInfo(const char* nameStr, TypeInfo* tiPtr)
	{
		if (!tiPtr){
			tiPtr = &Object::TypeInfo;
		}

		TypeInfo* retPtr = nullptr;

		while (tiPtr){
			if (strcmp(tiPtr->getTypeName(), nameStr) == 0){
				retPtr = tiPtr;
				break;
			}

			if (tiPtr->getChild()){
				retPtr = findTypeInfo(nameStr, tiPtr->getChild());
				if (retPtr){
					break;
				}
			}

			tiPtr = tiPtr->getNext();
		}

		return retPtr;
	}

	void TypeInfo::dump(TypeInfo* tiPtr)
	{
		if (!tiPtr){
			tiPtr = &Object::TypeInfo;
		}

		while (tiPtr){
			int parentCount = 0;
			auto* tiPtrParent = tiPtr->getParent();
			while (tiPtrParent){ 
				parentCount++; 
				tiPtrParent = tiPtrParent->getParent();
			}
			while (parentCount > 0){
				LUNA_TRACE(L"%s", L" ");
				if (--parentCount == 0){
					LUNA_TRACE(L"%s", L"+");
				}
			}
			LUNA_TRACELINE(L"%hs", tiPtr->getTypeName());
			if (tiPtr->getChild()){
				dump(tiPtr->getChild());
			}
			tiPtr = tiPtr->mNextPtr;
		}
	}
}