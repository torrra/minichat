#pragma once

#ifdef UNICODE
#undef UNICODE
#undef _UNICODE
#endif

#if 1
#define UNICODE
#define _UNICODE
#endif

namespace net
{
	// Format string and print to console
    void	consoleOutput(const char* format, ...);

	// Set console codepage
	void	initConsole();
}