#include <Network/Network.h>
#include <Network/Packet.h>
#include <Network/Server.h>
#include <Network/ConsoleOutput.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "ServerApp.h"

static int serverMain(void)
{
	// startup WSA and server app
	int						errorStartup = net::startup();
    server::ServerApp       serverApp;

    while(serverApp.isRunning())
    {
		// update incoming message list and
		// send them to clients
		serverApp.updateServer();
    }

	// disconnect clients and close app
	serverApp.server().terminate();
	net::cleanup();
	return errorStartup;
}

// Look for memory leaks in debug mode only
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
	SetConsoleTitleA("Mini chat Server");

	// Memory states to look for memory leaks
	_CrtMemState	start;
	_CrtMemState	end;

	_CrtMemCheckpoint(&start);

	// Main application function
	int		returnValue = serverMain();

	_CrtMemCheckpoint(&end);
	printMemory(&start, &end);

	return returnValue;
}

#else

int main(void)
{
	SetConsoleTitleA("Mini chat Server");

	// Main application function
	return serverMain();
}

#endif // !NDEBUG