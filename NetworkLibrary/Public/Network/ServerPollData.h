#pragma once

#include "Socket.h"

namespace net
{
    enum ServerPollResult : unsigned char
    {
        NONE,
        NEW_CONNECTION,
        NEW_DATA,
        CONNECTION_LOST
    };

    struct ServerPollData
    {
        Socket::Handle_t    m_socket;
        ServerPollResult    m_status;
    };
}