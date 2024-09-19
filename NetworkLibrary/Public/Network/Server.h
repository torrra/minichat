#pragma once

#include "Socket.h"
#include "FileDescriptor.h"
#include "ServerPollData.h"

#include <vector>

namespace net
{
    class Server
    {
    public:
                                Server(void);

        ServerPollData          serverUpdate(void);
        Socket                  getSocket(void)         const;

    private:

        Socket                  acceptConnection(void);

        FileDescriptorSet       m_clients;
        Socket                  m_socket;
    };
}