#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <WinSock2.h>
#include <stdio.h>

#include "Server.h"
#include "ErrorHandling.h"
#include "ConsoleOutput.h"

namespace net
{

    Server::Server(void)
    {
        initConsole();
        m_socket.createServer(NET_DEFAULT_PORT);

        m_clients.push_back(
        {
            .m_socket = m_socket.getHandle(),
            .m_requestedEvents = POLLRDNORM,
            .m_returnedEvents = 0
        });
    }

    Server::~Server(void)
    {
        for (SocketEvent client : m_clients)
        {
            Socket clientSock(client.m_socket);

            clientSock.shutdown();
            clientSock.close();
        }

        m_socket.shutdown();
        m_socket.close();
    }

    int Server::serverUpdate(void)
    {
        std::vector<SocketEvent>    clientCopy = m_clients;
        pollfd*                     copiedData = (pollfd*) clientCopy.data();
        int                         timeoutMs = static_cast<int>(INFINITE);

        int result = WSAPoll(copiedData,
                             static_cast<int>(clientCopy.size()),
                             timeoutMs);

        if (result == SOCKET_ERROR)
            reportWindowsError("WSAPoll", WSAGetLastError());

        else
        {
           checkListener(clientCopy[0]);
           checkClients(clientCopy);
        }

        return result;
    }

    Socket Server::getSocket(void) const
    {
        return m_socket;
    }

    const std::vector<Socket>& Server::getIncoming() const
    {
        return m_incomingQueue;
    }

    const std::vector<Socket>& Server::getOutgoing() const
    {
        return m_outgoingQueue;
    }

    std::vector<Packet> Server::receiveAllPackets(void)
    {
        std::vector<Packet>     receivedPackets;
        char                    receiveBuffer[NET_MAX_PACKET_SIZE];
        size_t                  size;


        for (const Socket& client : m_incomingQueue)
        {
            memset(receiveBuffer, 0, sizeof receiveBuffer);

            m_socket.receiveFrom(client, receiveBuffer, sizeof receiveBuffer);
            size = Packet::findPacketSize(receiveBuffer);

            receivedPackets.emplace_back(receiveBuffer, size, client.getHandle());
        }

        m_incomingQueue.clear();
        return receivedPackets;
    }

    void Server::sendAllPackets(void)
    {
        for (const Socket& client : m_outgoingQueue)
        {
            for (const Packet& packet : m_outgoingPackets)
            {
                if (packet.getSender() == client)
                    continue;

                m_socket.sendTo(client, packet.getData(),
                                static_cast<int>(packet.getSize()));

            }
        }

        m_outgoingPackets.clear();
        m_outgoingQueue.clear();
    }

    Packet& Server::createPacket(void* data, size_t size)
    {
        return m_outgoingPackets.emplace_back(data, size, m_socket);
    }

    void Server::addPacket(const Packet& packet)
    {
        m_outgoingPackets.push_back(packet);
    }



    Socket Server::acceptConnection(void)
    {
        std::string     connectionMessage = "Successfully connected.\r\n";
        int             stringSize = static_cast<int>(connectionMessage.size()) + 1;

        Socket          accepted = m_socket.accept();

        if (!accepted.isValid())
            return Socket(INVALID_SOCKET);

        m_socket.sendTo(accepted, connectionMessage.c_str(), stringSize);

        return accepted;
    }

    Socket Server::checkListener(const SocketEvent& listener)
    {
        if (!(listener.m_returnedEvents & POLLRDNORM))
            return Socket(INVALID_SOCKET);


        Socket  accepted = m_socket.accept();

        if (accepted.getHandle() == INVALID_SOCKET)
            return accepted;

        m_clients.push_back(
        {
            .m_socket = accepted.getHandle(),
            .m_requestedEvents = POLLRDNORM,
            .m_returnedEvents = 0
        });

        return accepted;
    }

    void Server::checkClients(std::vector<SocketEvent>& clients)
    {
        m_incomingQueue.clear();
        m_outgoingQueue.clear();

        for (size_t index = clients.size() - 1ull; index > 0ull; --index)
        {
            SocketEvent&    clientEvents = clients[index];

            if (checkInvalidEvents(clientEvents.m_returnedEvents))
            {
                Socket toClose(clientEvents.m_socket);

                toClose.shutdown();
                toClose.close();

                m_clients.erase(m_clients.begin() + index);
                continue;
            }

            if (clientEvents.m_returnedEvents & POLLRDNORM)
                m_incomingQueue.emplace_back(clientEvents.m_socket);

            //if (clientEvents.m_returnedEvents & POLLWRNORM)
            m_outgoingQueue.emplace_back(clientEvents.m_socket);

        }
    }

    bool Server::checkInvalidEvents(short events)
    {
        if (events & POLLERR)
            return true;

        if (events & POLLHUP)
            return true;

        if (events & POLLNVAL)
            return true;

        return false;
    }

}
