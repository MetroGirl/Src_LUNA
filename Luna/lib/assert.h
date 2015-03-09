//
// assertion.
//

#ifndef LUNA_ASSERT_H_INCLUDED
#define LUNA_ASSERT_H_INCLUDED

//! @brief アサーション
#if !LUNA_PUBLISH
#define LUNA_ASSERT(expression_, message_) \
do {\
	if (!(expression_)){\
		if (luna::is_same<decltype(message_), char*>::value){\
			LUNA_TRACE(L"assertion failed! \r\n %s", message_); \
		}else{\
			LUNA_TRACE(L"assertion failed! \r\n %ls", message_); \
		}\
		LUNA_DEBUG_BREAK(); \
	}\
} while (0)
#else
#define LUNA_ASSERT(expression_, message_) do {} while (0)
#endif

#define LUNA_ABORT(...) \
do {\
	LUNA_TRACE(__VA_ARGS__); \
	\
	{\
		c16 buffer[2048] = {};\
		swprintf(buffer, __VA_ARGS__);\
		::FatalAppExitW(0, buffer);\
	}\
} while (0)


#endif // LUNA_ASSERT_H_INCLUDED