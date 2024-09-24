#include <Network/ConsoleOutput.h>

#include "MessageHandling.h"

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
                running = false;
                net::consoleOutput("Shutting down server...\n");
                server.terminate();
                return;
            }

            net::consoleOutput(message.c_str());
            server.addPacket(received);
        }
        server.sendAllPackets();
	}
}
