//
// Lua
//

#ifndef LUNA_RESOURCE_LUA_H_INCLUDED
#define LUNA_RESOURCE_LUA_H_INCLUDED

#include "lib/type.h"
#include "lib/resourceObject.h"
#include "lib/scriptContext.h"

namespace luna{
	class ResourceLua : public ResourceObject
	{
		LUNA_DECLARE_CONCRETE(ResourceLua, ResourceObject);

	public:
		ResourceLua();
		virtual ~ResourceLua();

		bool load(class Stream&) override;
		bool isLoadable(class Stream&) override;
		void freeResource() override;

		ScriptContext& getContext()
		{
			return mContext;
		}

	protected:
		ScriptContext mContext;
	};
}

#endif /* LUNA_RESOURCE_LUA_H_INCLUDED */