#include <Network/ConsoleOutput.h>

#include "MessageHandling.h"

#ifndef SHUTDOWN_MSG
#define SHUTDOWN_MSG "jrII0JB0y6"
#endif // !NET_SHUTDOWN_MSG


namespace server
{
	void processMessages(net::Server& server, bool& running)
	{
        std::vector<net::Packet>    packets = server.receiveAllPackets();

        for (const net::Packet& received : packets)
        {
            std::string     message = net::Packet::unpackMessage(received);

            if (message.find("/system shutdown") != std::string::npos)
            {
                size_t      msgLen = strnlen_s(SHUTDOWN_MSG, NET_MAX_PACKET_SIZE);

                running = false;
                net::consoleOutput("Shutting down server...\n");
                server.createPacket(SHUTDOWN_MSG, msgLen);
            }

            net::consoleOutput(message.c_str());
            server.addPacket(received);
        }
        server.sendAllPackets();
	}
}
