//
//
//

#include "stdafx.h"
#include "resourceLua.h"
#include "lib/scriptManager.h"

namespace luna
{
	LUNA_IMPLEMENT_ABSTRACT(luna::ResourceLua);

	ResourceLua::ResourceLua()
		: base_t()
	{
	}

	ResourceLua::~ResourceLua()
	{
	}

	bool ResourceLua::load(class Stream& fs)
	{
		auto buffer = fs.readByteString();
		if (luaL_loadbuffer(mContext, buffer.data(), buffer.size(), "") || lua_pcall(mContext, 0, 0, 0)){
			LUNA_ERRORLINE(L"%hs", lua_tostring(mContext, -1));
			return false;
		}
		return true;
	}

	bool ResourceLua::isLoadable(class Stream& fs)
	{
		if (fs.getExtension() != L"lua"){
			return false;
		}

		auto buffer = fs.readByteString();

		ScriptContext context;
		luaL_loadbuffer(context, buffer.data(), buffer.size(), "tmp");
		if (lua_pcall(context, 0, 0, 0)) {
			LUNA_ERRORLINE(L"%hs", lua_tostring(context, -1));
			return false;
		}

		lua_getglobal(context, "ID");
		const c8* idStr = lua_tostring(context, -1);
		if (!idStr || strcmp(idStr, "Script") != 0){
			return false;
		}
		return true;
	}

	void ResourceLua::freeResource()
	{
		mContext.reset();
	}
}
