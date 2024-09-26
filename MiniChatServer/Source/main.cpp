#include <Network/Network.h>
#include <Network/Packet.h>
#include <Network/Server.h>
#include <Network/ConsoleOutput.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "MessageHandling.h"

int serverMain(void)
{
	int						errorStartup = net::startup();
    bool					running = true;
    server::ServerData		serverApp;

    while(running)
    {
		serverApp.m_server.serverUpdate();
		server::processMessages(serverApp, running);
    }

	serverApp.m_server.terminate();
	net::cleanup();
	return errorStartup;
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