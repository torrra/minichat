#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Ws2tcpip.h>

#include "Network.h"
#include "Socket.h"
#include "ErrorHandling.h"
#include "ConsoleOutput.h"

#define WSA_MAJOR_VER   2
#define WSA_MINOR_VER   2

namespace net
{

    int startup(void)
    {
        // Start up WinSock2 API
        WSADATA     startupData;
        int	        error = WSAStartup(MAKEWORD(WSA_MAJOR_VER, WSA_MINOR_VER),
                                        &startupData);

        if (error)
	    {
            // Display error on failure
			reportWindowsError("WSAStartup", error);
			return error;
		}

        return error;

    }

    int cleanup(void)
    {
        // Cleanup WinSock2
		int     error = WSACleanup();

		if (SOCKET_ERROR == error)
		{
            // Display error on failure
            reportWindowsError("WSACleanup", error);
			return error;
		}

        return error;
    }


}