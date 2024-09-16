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

void network(void)
{
    /*nlib::Socket a, b;

    a.createServerSocket("127.0.0.1", "doom");
    b.createClientSocket("127.0.0.1", "doom");

    b.connectToServer(a);

    b.testSendToServer(a);
    a.testSendToCLient(b);*/



    platform();
}

#include "Constants.h"

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
			OutputDebugString(TEXT("[ERROR] Unable to startup WSA.\n"));
			return NetResult::NET_WSA_STARTUP_ERROR;
		}

        return NetResult::NET_NO_ERROR;

    }

    int cleanup(void)
    {
		int error = WSACleanup();

		if (SOCKET_ERROR == error)
		{
			OutputDebugString(TEXT("[Error] WSACleanup\n"));
			return NetResult::NET_WSA_CLEANUP_ERROR;
		}

        return NetResult::NET_NO_ERROR;
    }


}