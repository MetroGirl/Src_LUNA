#include "stdafx.h"
#include "task.h"

namespace luna{
	LUNA_IMPLEMENT_ABSTRACT(luna::Task);

	Task::Task()
		: mScenePtr(nullptr)
		, mRenderPassPtr(nullptr)
	{
	}

	Task::~Task()
	{
	}
}

