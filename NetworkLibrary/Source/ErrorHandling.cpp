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
#include "ConsoleOutput.h"


namespace net
{

	void reportWindowsError(const char* context, int error)
	{
		LPSTR	buffer;
		int		formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
							  FORMAT_MESSAGE_FROM_SYSTEM |
						      FORMAT_MESSAGE_IGNORE_INSERTS;

	    int		charCount = FormatMessageA(formatFlags, nullptr, error, GetUserDefaultLangID(),
										  (LPSTR) &buffer, 0, nullptr);

		if (charCount)
		{
			consoleOutput("[ERROR] %1 : error %3!d!\n%2\n", context, buffer, error);
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
}