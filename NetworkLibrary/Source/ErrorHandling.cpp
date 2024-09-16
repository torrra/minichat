#ifdef UNICODE
#undef UNICODE
#undef _UNICODE
#endif // UNICODE

#if 0
#define UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdarg>

#include "ErrorHandling.h"
#include "Constants.h"


namespace net
{

	void reportWSAError(const char* context, int error)
	{
		LPSTR	buffer;
		int		formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
							  FORMAT_MESSAGE_FROM_SYSTEM |
						      FORMAT_MESSAGE_IGNORE_INSERTS;

	    int		charCount = FormatMessageA(formatFlags, nullptr, error, GetUserDefaultLangID(),
										  (LPSTR) &buffer, 0, nullptr);

		if (charCount)
		{
			consoleOutput("[ERROR] %1 : %2\n", context, buffer);
			LocalFree(buffer);
		}
		else
		{
			int		formatFailure = GetLastError();

			OutputDebugStringA("FormatMessage error");
			DebugBreak();
			(void)formatFailure;

		}

	}


	void consoleOutput(const char* format, ...)
	{
		LPTSTR		buffer;
		va_list		args;
		int			formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_STRING;

		va_start(args, format);

		int		charCount = FormatMessageA(formatFlags, (LPTSTR)format, 0,
			GetUserDefaultLangID(),
			(LPTSTR)&buffer, 0, &args);

		if (charCount)
		{
			void* handle = GetStdHandle(STD_OUTPUT_HANDLE);

			if (!WriteConsoleA(handle, buffer, charCount, nullptr, 0))
			{
				int		error = GetLastError();

				OutputDebugStringA("WriteConsole format failure\n");
				(void)error;

			}

			LocalFree(buffer);

		}
		else
		{
			int		error = GetLastError();

			OutputDebugStringA("FormatMessage format failure\n");
			DebugBreak();
			(void)error;
		}


	}

	void initConsole()
	{
		int		dwCodePage;
		int		localeFlags = LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER;
		int		success = GetLocaleInfoEx(L"Fr-FR", localeFlags,
			(LPWSTR)&dwCodePage,
			sizeof dwCodePage / sizeof WCHAR);

		if (success)
			SetConsoleOutputCP(dwCodePage);
		else
			reportWSAError("initConsole", GetLastError());
	}

}