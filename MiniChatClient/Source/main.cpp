#include <Network/Network.h>
#include <Network/Socket.h>

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

	return returnValue;
}