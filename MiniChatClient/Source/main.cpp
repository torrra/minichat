#include <Network/Network.h>
#include <Network/Socket.h>

#include <WinSock2.h>

#include <iostream>

int main(void)
{
	net::startup();

    net::Socket client;

    //client.create("10.207.0.59", "8888", false);
    //
    //client.create("172.27.192.1", "8888", false);
    client.createClient("192.168.2.53", "8888");
    //client.createClient("10.207.0.93", "3983");

    //client.createClient("10.207.0.59", "8888");

    char buffer[6];

    bool debug = true;

    int bytes = client.receive(buffer, 6);
   //int bytes = client.send(236, "Testing...\0", 12);

  /*  if (bytes == -1)
        std::cout << WSAGetLastError() << "\n";*/

    std::cout << "bytes received: " << bytes << "\nbuffer : " << buffer << std::endl;

    while (debug);

    net::cleanup();

	return 0;
}