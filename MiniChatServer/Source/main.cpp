#include <Network/Network.h>
#include <Network/Packet.h>
#include <Network/Server.h>

#include <iostream>

int main(void)
{
	int errorStartup = net::startup();
    bool running = true;

    net::Server server;

    while(running)
    {
       server.serverUpdate();

       auto packets = server.receiveAllPackets();

       for (const net::Packet& received : packets)
       {
           std::cout << net::Packet::unpackMessage(received) << '\n';

           server.addPacket(received);
       }

       server.sendAllPackets();

    }

	net::cleanup();

	return errorStartup;
}