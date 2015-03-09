//
// Script.
//

#ifndef LUNA_SCRIPT_MANAGER_H_INCLUDED
#define LUNA_SCRIPT_MANAGER_H_INCLUDED

#include "lib/object.h"
#include "lib/singleton.h"
#include "lib/scriptContext.h"

namespace luna{
	//! @brief スクリプトマネージャ
	//! 
	//! スクリプトの適切な管理と実行を行います
	//!
	class ScriptManager : public Object, public Singleton<ScriptManager>
	{
		LUNA_DECLARE_CONCRETE(ScriptManager, Object);

	public:
		ScriptManager();
		~ScriptManager();

		void initialize();
		void postUpdate();
		void finalize();

	private:
	};
}

#endif // LUNA_SCRIPT_MANAGER_H_INCLUDED
