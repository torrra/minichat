#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#include "Network.h"
#include "Constants.h"

// TODO: REMOVE
#include "Platform.h"
#include "Socket.h"
#include "ErrorHandling.h"

void network(void)
{
    net::initConsole();
    platform();
}


namespace net
{

    int startup(void)
    {
        WSADATA startupData;
        int	    error = WSAStartup(
                MAKEWORD(2, 2),
                &startupData
        );

        if (error)
	    {
			reportWSAError("WSAStartup", error);
			return error;
		}

        return error;

    }

    int cleanup(void)
    {
		int error = WSACleanup();

		if (SOCKET_ERROR == error)
		{
            reportWSAError("WSACleanup", error);
			return error;
		}

        return error;
    }


}