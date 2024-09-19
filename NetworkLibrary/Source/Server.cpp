#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <WinSock2.h>
#include <stdio.h>

#include "Server.h"
#include "ErrorHandling.h"
//#include "FileDescriptor.h"

namespace net
{

    Server::Server(void)
    {
        m_socket.createServer(NET_DEFAULT_PORT);

        //memset(&m_clients, 0, sizeof m_clients);

        FD_ZERO(&m_clients);
        FD_SET(m_socket.getHandle(), &m_clients);
    }

    ServerPollData Server::serverUpdate(void)
    {
        fd_set          fdCopy = (*reinterpret_cast<fd_set*>(&m_clients));
        TIMEVAL         timeout = { .tv_sec = 0l, .tv_usec = 1000l };
        ServerPollData  polledData
        {
            .m_socket = this->m_socket.getHandle(),
            .m_status = NONE
        };

        int         selectedCount = select(0, &fdCopy, nullptr, nullptr, &timeout);

        if (selectedCount == SOCKET_ERROR)
            reportWSAError("::select", WSAGetLastError());

        for (int socketNum = 0; socketNum < selectedCount; ++socketNum)
        {
            SOCKET incoming = fdCopy.fd_array[socketNum];

            if (incoming == m_socket.getHandle())
            {
                polledData.m_status = NEW_CONNECTION;
                polledData.m_socket = acceptConnection().getHandle();
            }
            else
            {
                polledData.m_status = NEW_DATA;
                polledData.m_socket = incoming;
            }
        }

        return polledData;
    }

    Socket Server::getSocket(void) const
    {
        return m_socket;
    }


    Socket Server::acceptConnection(void)
    {
        std::string     connectionMessage = "Successfully connected.\r\n";
        int             stringSize = static_cast<int>(connectionMessage.size()) + 1;

        Socket          accepted = m_socket.accept();

        if (!accepted.isValid())
            return Socket(INVALID_SOCKET);

        FD_SET(accepted.getHandle(), &m_clients);
        m_socket.sendTo(accepted, connectionMessage.c_str(), stringSize);

        return accepted;
    }


}
