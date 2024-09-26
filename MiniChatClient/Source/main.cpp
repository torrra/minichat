#include <Network/Network.h>
#include <Network/Socket.h>
#include <Network/ConsoleOutput.h>

#include "ClientApp.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static int clientMain(void)
{
	int		error = net::startup();
    bool	running = true;

	if (error)
		return error;

    client::ClientApp   client;

	// ask user to enter IP
    client.welcomeMessage();

	// run until
    while (running)
        client.update(running);

	return net::cleanup();
}

// Only look for memory leaks in debug mode
#ifndef NDEBUG
static void printMemory(_CrtMemState* start, _CrtMemState* end)
{
	// difference between start end end state
	_CrtMemState	diff;

	// print memory information
	if (_CrtMemDifference(&diff, start, end))
	{
		OutputDebugString(TEXT(" ------ _CrtMemDumpStatistics ------ "));
		_CrtMemDumpStatistics(&diff);

		OutputDebugString(TEXT(" ------ _CrtMemDumpAllObjectsSince ------ "));
		_CrtMemDumpAllObjectsSince(start);

		OutputDebugString(TEXT(" ------ _CrtDumpMemoryLeaks ------"));
		_CrtDumpMemoryLeaks();
	}
}

int main(void)
{
	// Memory states to look for memory leaks
	_CrtMemState	start;
	_CrtMemState	end;

	_CrtMemCheckpoint(&start);

	// Main application function
	int		returnValue = clientMain();

	_CrtMemCheckpoint(&end);
	printMemory(&start, &end);

	net::consoleOutput("Press enter to exit.\n");
	(void) getchar();

	return returnValue;
}

#else

int main(void)
{
	// Main application function
	int		returnValue = clientMain();

	net::consoleOutput("Press enter to exit.\n");
	(void)getchar();

	return returnValue;
}

#endif // !NDEBUG