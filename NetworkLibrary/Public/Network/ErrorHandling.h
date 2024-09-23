#pragma once

namespace net
{
	// Print error message associated with windows error code
	void	reportWindowsError(const char* context, int error);
}