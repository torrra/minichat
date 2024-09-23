#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ConsoleOutput.h"
#include "ErrorHandling.h"

namespace net
{
    void consoleOutput(const char* format, ...)
	{
		int			dwCodepage = 0;
		char*		buffer;
		va_list		args;
		int			formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
								  FORMAT_MESSAGE_FROM_STRING;

		va_start(args, format);

		int		charCount = FormatMessageA(formatFlags, (char*)format, 0,
										   dwCodepage,
										   (char*)&buffer, 0, &args);

		if (charCount)
		{
			void* handle = GetStdHandle(STD_OUTPUT_HANDLE);

			if (!WriteConsoleA(handle, buffer, charCount, nullptr, 0))
			{
				int		error = GetLastError();

				OutputDebugStringA("WriteConsole format failure\n");
				error = error;

			}

			LocalFree(buffer);

		}
		else
		{
			int		error = GetLastError();

			OutputDebugStringA("FormatMessage format failure\n");
			DebugBreak();
			error = error;
		}


	}

	void initConsole()
	{
		int		dwCodePage;
		int		localeFlags = LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER;
		int		success = GetLocaleInfoEx(L"en-US", localeFlags,
			(LPWSTR)&dwCodePage,
			sizeof dwCodePage / sizeof WCHAR);

		if (success)
		{
			SetConsoleOutputCP(dwCodePage);
			SetConsoleCP(dwCodePage);
		}
		else
			reportWindowsError("initConsole", GetLastError());
	}
}