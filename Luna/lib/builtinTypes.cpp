#include "stdafx.h"
#include "builtinTypes.h"

namespace luna{
#if LUNA_BITWIDTH_32
	static_assert(sizeof(void*) == 4, "");
#else
	static_assert(sizeof(void*) == 8, "");
#endif
}