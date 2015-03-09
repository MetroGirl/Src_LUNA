#include "stdafx.h"
#include "scriptManager.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::ScriptContext);

	ScriptContext::ScriptContext()
		: base_t()
		, mContextPtr(nullptr)
	{
		open();
	}

	ScriptContext::~ScriptContext()
	{
		close();
	}

	void ScriptContext::open()
	{
		mContextPtr = luaL_newstate();
		luaL_openlibs(mContextPtr);
	}

	void ScriptContext::close()
	{
		lua_close(mContextPtr);
	}

	void ScriptContext::reset()
	{
		close();
		open();
	}

	void ScriptContext::dumpStack()
	{
#if !LUNA_PUBLISH
		const int num = lua_gettop(*this);
		if (num == 0) {
			LUNA_TRACE(L"No stack.\n");
			return;
		}
		for (int i = num; i >= 1; i--) {
			LUNA_TRACE(L"%03d(%04d): ", i, -num + i - 1);
			int type = lua_type(*this, i);
			switch (type) {
			case LUA_TNIL:
				LUNA_TRACE(L"NIL\n");
				break;
			case LUA_TBOOLEAN:
				LUNA_TRACE(L"BOOLEAN %s\n", lua_toboolean(*this, i) ? "true" : "false");
				break;
			case LUA_TLIGHTUSERDATA:
				LUNA_TRACE(L"LIGHTUSERDATA\n");
				break;
			case LUA_TNUMBER:
				LUNA_TRACE(L"NUMBER %f\n", lua_tonumber(*this, i));
				break;
			case LUA_TSTRING:
				LUNA_TRACE(L"STRING %s\n", lua_tostring(*this, i));
				break;
			case LUA_TTABLE:
				LUNA_TRACE(L"TABLE\n");
				break;
			case LUA_TFUNCTION:
				LUNA_TRACE(L"FUNCTION\n");
				break;
			case LUA_TUSERDATA:
				LUNA_TRACE(L"USERDATA\n");
				break;
			case LUA_TTHREAD:
				LUNA_TRACE(L"THREAD\n");
				break;
			}
		}
		LUNA_TRACE(L"-----------------------------\n\n");
#endif
	}

	void ScriptContext::invokeMain()
	{
		lua_getglobal(mContextPtr, "main");
		if (lua_pcall(mContextPtr, 0, 0, 0) != 0){
			LUNA_ERRORLINE(L"%hs", lua_tostring(mContextPtr, -1));
		}
	}
}
