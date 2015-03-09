#ifndef LUNA_EXTERNAL_H_INCLUDED
#define LUNA_EXTERNAL_H_INCLUDED

// 外部ライブラリのインポートを、ここに
namespace luna{
	//using namespace glm;
}

#include "lua.hpp"
#include "lauxlib.h"
#include "lualib.h"
#include "bass.h"

#if LUNA_DEBUG
#pragma comment(lib, "luad.lib")
#else
#pragma comment(lib, "lua.lib")
#endif

#pragma comment(lib, "bass.lib")

#endif // LUNA_EXTERNAL_H_INCLUDED