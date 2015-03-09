//
// standard libraries.
//

#ifndef LUNA_STD_H_INCLUDED
#define LUNA_STD_H_INCLUDED

#if LUNA_COMPILER_MSC
#define _CRT_SECURE_NO_WARNINGS// we don't want to use non-standard _s() functions.
#endif

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <clocale>

#include <type_traits>
#include <vector>
#include <map>
#include <hash_map>
#include <string>
#include <memory>
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <set>

namespace luna{
	using std::move;

	using std::is_same;

	using std::vector;
	using std::wstring;
	using std::map;
	using std::hash_map;
	using std::unique_ptr;
	using std::make_unique;

	using std::all_of;
	using std::any_of;
	using std::none_of;

	using std::array;

	using std::getline;
	using std::istringstream;
	using std::pair;
	using std::string;

	using std::min;
	using std::max;

	using std::function;
}

#endif // LUNA_STD_H_INCLUDED
