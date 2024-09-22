#include <Network/Network.h>
#include <Network/Socket.h>

#include "ClientApp.h"

#include <iostream>

int main(void)
{
	net::startup();

    client::ClientApp   client;
    client.welcomeMessage();

    bool debug = true;


    while (debug)
    {
        client.update();
    }

    net::cleanup();

	return 0;
}