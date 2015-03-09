#include "stdafx.h"
#include "debug.h"

namespace luna{
	LUNA_IMPLEMENT_SINGLETON(Debug);

#if !LUNA_PUBLISH
	void Debug::allocateConsole()
	{
#if LUNA_WINDOWS
		::AllocConsole();

		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$", "r", stdin);

		setlocale(LC_ALL, "");
#endif
	}

	void Debug::freeConsole()
	{
#if LUNA_WINDOWS
		::FreeConsole();
#endif
	}

	void Debug::trace(const c16* format, ...)
	{
		va_list ap;

		c16 buffer[2048] = {};

		va_start(ap, format);
		vswprintf(buffer, format, ap);
		va_end(ap);

		LUNA_INTRINSIC_SET_PRINT_COLOR_WHITE();
		LUNA_INTRINSIC_PRINT(buffer);
	}

	void Debug::traceLine(const c16* format, ...)
	{
		va_list ap;

		c16 buffer[2048] = {};

		va_start(ap, format);
		vswprintf(buffer, format, ap);
		va_end(ap);

		wcscat(buffer, L"\r\n");

		LUNA_INTRINSIC_SET_PRINT_COLOR_WHITE();
		LUNA_INTRINSIC_PRINT(buffer);
	}

	void Debug::warning(const c16* format, ...)
	{
		va_list ap;

		c16 buffer[2048] = {};

		va_start(ap, format);
		vswprintf(buffer, format, ap);
		va_end(ap);

		LUNA_INTRINSIC_SET_PRINT_COLOR_YELLOW();
		LUNA_INTRINSIC_PRINT(buffer);
	}

	void Debug::warningLine(const c16* format, ...)
	{
		va_list ap;

		c16 buffer[2048] = {};

		va_start(ap, format);
		vswprintf(buffer, format, ap);
		va_end(ap);

		wcscat(buffer, L"\r\n");

		LUNA_INTRINSIC_SET_PRINT_COLOR_YELLOW();
		LUNA_INTRINSIC_PRINT(buffer);
	}

	void Debug::error(const c16* format, ...)
	{
		va_list ap;

		c16 buffer[2048] = {};

		va_start(ap, format);
		vswprintf(buffer, format, ap);
		va_end(ap);

		LUNA_INTRINSIC_SET_PRINT_COLOR_RED();
		LUNA_INTRINSIC_PRINT(buffer);
	}

	void Debug::errorLine(const c16* format, ...)
	{
		va_list ap;

		c16 buffer[2048] = {};

		va_start(ap, format);
		vswprintf(buffer, format, ap);
		va_end(ap);

		wcscat(buffer, L"\r\n");

		LUNA_INTRINSIC_SET_PRINT_COLOR_RED();
		LUNA_INTRINSIC_PRINT(buffer);
	}
#endif
}