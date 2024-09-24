#pragma once

#include "Socket.h"
#include "FileDescriptor.h"
#include "Packet.h"

#include <vector>

namespace net
{
    class Server
    {
    public:

        using SocketArray = std::vector<Socket>;

                                Server(void);
                                ~Server(void);

        int                     serverUpdate(void);

        std::vector<Packet>     receiveAllPackets(void);
        void                    sendAllPackets(void);

        Packet&                 createPacket(const void* data, size_t size);
        void                    addPacket(const Packet& packet);
        void                    terminate(void);

        // Accessors

        Socket                  getSocket(void)         const;
        const SocketArray&      getIncoming()           const;
        const SocketArray&      getOutgoing()           const;

    private:

        // Check for incoming connections
        Socket                  checkListener(const SocketEvent& listener);

        // Check if connected clients sent data
        void                    checkClients(std::vector<SocketEvent>& clients);

        // Check if a client is disconnected
        bool                    checkInvalidEvents(short events);

        // Connected clients
        std::vector<SocketEvent>    m_clients;

        // Client sockets that sent data to server
        std::vector<Socket>         m_incomingQueue;

        // Client sockets that are able to receive next packet
        std::vector<Socket>         m_outgoingQueue;

        // Next packet to be sent to clients
        std::vector<Packet>         m_outgoingPackets;

        // Listening socket handle
        Socket                      m_socket;
    };
}