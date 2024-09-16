#pragma once

namespace net
{
	void	reportWSAError(const char* context, int error);
	void	consoleOutput(const char* format, ...);
	void	initConsole();
}