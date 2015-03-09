#include "stdafx.h"
#include "scriptManager.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::ScriptManager);
	LUNA_IMPLEMENT_SINGLETON(luna::ScriptManager);

	ScriptManager::ScriptManager()
		: base_t()
	{
	}

	ScriptManager::~ScriptManager()
	{
	}

	void ScriptManager::initialize()
	{
	}

	void ScriptManager::postUpdate()
	{
	}

	void ScriptManager::finalize()
	{
	}
}
