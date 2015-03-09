//
// intrinsics.
//

#ifndef LUNA_INTRINSIC_H_INCLUDED
#define LUNA_INTRINSIC_H_INCLUDED

#if LUNA_WINDOWS
//! @brief トラップ命令の発行
#define LUNA_INTRINSIC_DEBUG_BREAK()\
	DebugBreak()

#define LUNA_INTRINSIC_SET_PRINT_COLOR_WHITE()\
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#define LUNA_INTRINSIC_SET_PRINT_COLOR_YELLOW()\
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

#define LUNA_INTRINSIC_SET_PRINT_COLOR_RED()\
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY)

//! @brief 文字列のコンソール出力
#define LUNA_INTRINSIC_PRINT(message_)\
	OutputDebugStringW(message_); \
	wprintf(L"%s", message_)

#define LUNA_INTRINSIC_SLEEP(msec_)\
	Sleep(msec_)
#endif

//! @brief up align.
template<typename T>
T LUNA_ALIGN(T value, size_t alignment)
{
	return (value + (alignment - 1)) & ~(alignment - 1);
}

#define LUNA_TRACECODE(code_)\
	LUNA_TRACELINE(L"<--code: %hs", #code_);\
	(code_);\
	LUNA_TRACELINE(L"code-->: %hs", #code_);

#endif // LUNA_INTRINSIC_H_INCLUDED
