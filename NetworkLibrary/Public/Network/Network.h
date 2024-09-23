# pragma once

namespace net
{
    // Start up WinSock2 API
    int     startup(void);

    // Cleanup WinSock2
    int     cleanup(void);
}