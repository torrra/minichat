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
        // Set up console input/ouput codepage
        initConsole();

        // create listening socket and add to client list
        // (makes checking for updates easier)
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
        if (m_socket.isValid())
            terminate();
    }

    int Server::serverUpdate(void)
    {
        std::vector<SocketEvent>    clientCopy = m_clients;
        pollfd*                     copiedData = (pollfd*) clientCopy.data();
        int                         timeoutMs = static_cast<int>(INFINITE);

        // Check for events on listener and clients
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

    const std::vector<Socket>& Server::getInvalid(void) const
    {
        return m_invalidClients;
    }

    std::vector<Packet> Server::receiveAllPackets(void)
    {
        std::vector<Packet>     receivedPackets;
        char                    receiveBuffer[NET_MAX_PACKET_SIZE];
        size_t                  size;

        // Receive packet from all clients that have sent data
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
        // Send all messages to all available clients
        for (const Socket& client : m_outgoingQueue)
        {
            for (const Packet& packet : m_outgoingPackets)
            {
                // Do not send message back to sender
                if (packet.getSender() == client)
                    continue;

                m_socket.sendTo(client, packet.getData(),
                                static_cast<int>(packet.getSize()));

            }
        }

        m_outgoingPackets.clear();
        m_outgoingQueue.clear();
    }

    Packet& Server::createPacket(const void* data, size_t size)
    {
        return m_outgoingPackets.emplace_back(data, size, m_socket);
    }

    Packet& Server::createPacket(const void* data, size_t size, Socket sender)
    {
        return m_outgoingPackets.emplace_back(data, size, sender);
    }

    void Server::addPacket(const Packet& packet)
    {
        m_outgoingPackets.push_back(packet);
    }

    void Server::terminate(void)
    {
        // Close all sockets including listening socket
        for (SocketEvent& client : m_clients)
        {
            Socket      clientSock(client.m_socket);

            clientSock.close();
        }

       m_socket = Socket::INVALID_HANDLE;
    }

    Socket Server::checkListener(const SocketEvent& listener)
    {
        if (!(listener.m_returnedEvents & POLLRDNORM))
            return Socket(INVALID_SOCKET);


        Socket  accepted = m_socket.accept();

        if (accepted.getHandle() == INVALID_SOCKET)
            return accepted;

        // Add newly accepted client to client list
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
        m_invalidClients.clear();

        // Read all objects in client array except listening socket (index 0)
        for (size_t index = clients.size() - 1ull; index > 0ull; --index)
        {
            SocketEvent&    clientEvents = clients[index];

            // Terminate connection in socket is invalid
            if (checkInvalidEvents(clientEvents.m_returnedEvents))
            {
                Socket     toClose(clientEvents.m_socket);

                m_invalidClients.push_back(toClose);
                toClose.shutdown();
                toClose.close();

                m_clients.erase(m_clients.begin() + index);

                continue;
            }

            // Add to sockets to receive from if it has sent data
            // over to server
            if (clientEvents.m_returnedEvents & POLLRDNORM)
                m_incomingQueue.emplace_back(clientEvents.m_socket);

            m_outgoingQueue.emplace_back(clientEvents.m_socket);

        }
    }

    bool Server::checkInvalidEvents(short events)
    {
        // Check if any invalid flags was set

        if (events & POLLERR)
            return true;

        if (events & POLLHUP)
            return true;

        if (events & POLLNVAL)
            return true;

        return false;
    }

}
