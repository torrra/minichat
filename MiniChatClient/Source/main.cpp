#include <Network/Network.h>
#include <Network/Socket.h>

#include <WinSock2.h>

#include <iostream>

int main(void)
{
	net::startup();

    net::Socket client;

    client.create("172.20.144.1", "doom", false);

    char buffer[6];

    bool debug = true;

    int bytes = client.receive(buffer, 6);

    if (bytes == -1)
        std::cout << WSAGetLastError() << "\n";

    std::cout << "bytes received: " << bytes << "\nbuffer : " << buffer << std::endl;

    while (debug);

    net::cleanup();

	return 0;
}