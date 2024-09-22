#pragma once

#ifndef NETMAXSOCKETS
#define NET_MAX_SOCKETS     64
#endif

#include <vector>

#include "Socket.h"

namespace net
{
    // size = 104B, padding = 4B between count and sockets

    // used for select function
    struct FileDescriptorSet
    {
        // same memory layout as fd_set struct
        unsigned int           m_count;
        Socket::Handle_t       m_sockets[NET_MAX_SOCKETS];
    };


    // size = 16B, padding = 4B

    // used for WSAPoll function
    struct SocketEvent
    {
        // same memory layout as pollfd struct
        Socket::Handle_t        m_socket;
        short                   m_requestedEvents;
        short                   m_returnedEvents;

    };


}
