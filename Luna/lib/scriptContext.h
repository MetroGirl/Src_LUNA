//
// Script.
//

#ifndef LUNA_SCRIPT_CONTEXT_H_INCLUDED
#define LUNA_SCRIPT_CONTEXT_H_INCLUDED

#include "lib/object.h"

namespace luna{
	class ScriptContext : public Object
	{
		LUNA_DECLARE_CONCRETE(ScriptContext, Object);

	public:
		ScriptContext();
		~ScriptContext();

		inline operator lua_State*()
		{
			return mContextPtr;
		}

		void reset();

		//! @brief デバッグ文字列でスタック内容をダンプする
		void dumpStack();

		// 'main'の呼び出し
		void invokeMain();

	private:
		void open();
		void close();

	private:
		lua_State* mContextPtr;
	};
}

#endif // LUNA_SCRIPT_CONTEXT_H_INCLUDED
