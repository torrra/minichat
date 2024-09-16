#include <Network/Network.h>
#include <Network/Server.h>

int main(void)
{

	int errorStartup = net::startup();

	network();
	 //SOCKET* a = (SOCKET*) nlib::createSocket("127.0.0.1", "doom", false);

	//nlib::

	// (void)errorStartup;
	//(void)a;

    net::Socket server, client;

    server.create("127.0.0.1", "doom", true);

    bool debug = true;


    while (debug)
    {
        net::Socket accepted = server.accept();

        if (accepted)
        {
            server.send(accepted, "Hello\0", 6);
        }
    }

	net::cleanup();

	return errorStartup;
}