#pragma once

#include "Socket.h"
#include "FileDescriptor.h"
#include "Packet.h"
//#include "IPData.h"

#include <vector>

namespace net
{
    // size = 104B, no padding

    class Server
    {
    public:

        using SocketArray = std::vector<Socket>;

                                Server(void);
                                ~Server(void);

        int                     serverUpdate(void);
        Socket                  getSocket(void)         const;

        const SocketArray&      getIncoming()           const;
        const SocketArray&      getOutgoing()           const;

        std::vector<Packet>     receiveAllPackets(void);
        void                    sendAllPackets(void);

        Packet&                 createPacket(void* data, size_t size);
        void                    addPacket(const Packet& packet);

        //IPData                  getLocalAddress(void);

    private:

        Socket                  acceptConnection(void);
        Socket                  checkListener(const SocketEvent& listener);
        void                    checkClients(std::vector<SocketEvent>& clients);
        bool                    checkInvalidEvents(short events);


        std::vector<SocketEvent>    m_clients;
        std::vector<Socket>         m_incomingQueue;
        std::vector<Socket>         m_outgoingQueue;
        std::vector<Packet>         m_outgoingPackets;
        Socket                      m_socket;
    };
}