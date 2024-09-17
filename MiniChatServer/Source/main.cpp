#include <Network/Network.h>
#include <Network/Server.h>

#include <WinSock2.h>
#include <iostream>

int main(void)
{

	int errorStartup = net::startup();

	network();
	 //SOCKET* a = (SOCKET*) nlib::createSocket("127.0.0.1", "doom", false);

	//nlib::

	// (void)errorStartup;
	//(void)a;

    net::Socket server, client;

    //server.create("0.0.0.0", "8888", true);

    server.createServer("8888");

    bool debug = true;

    char receivedData[12];

     net::Socket accepted = server.accept();

     auto handle = server;

     if (accepted)
     {
         //std::cout << server.getHandle() << '\n';

         //server.send(accepted, (void*)(unsigned long long) server.getHandle(), sizeof(void*));
         server.send(accepted, "Hello\0", 6);
         //server.receive(receivedData, 12);
         //recvfrom()

         recv(accepted.getHandle(), receivedData, sizeof receivedData, 0);
     }

     while (debug);

	net::cleanup();

	return errorStartup;
}