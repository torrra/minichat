#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream> // TODO:error handling
#include <string.h>

#include "Socket.h"
#include "IPData.h"


namespace net
{
	int  Socket::m_pendingConnectionCap = 5;

	Socket::Socket(Handle_t handle, IPVersion version)
		: m_handle(handle), m_ipVersion(version)
	{
	}

	Socket::~Socket(void)
	{
		this->close();
	}

	int Socket::create(const std::string& hostname,
		const std::string& port, bool server)
	{
		int			result;
		IPData		connectionData
		{
			.m_ipString = hostname,
			.m_ipAddress = inet_addr(hostname.c_str()),
			.m_ipVersion = this->m_ipVersion
		};

		if (isalpha(hostname[0]))
			connectionData.m_portNumber = (unsigned short)atoi(port.c_str());

		else
		{
			servent* servEntry = getservbyname(port.c_str(), nullptr);

			connectionData.m_portNumber = ntohs(servEntry->s_port);
		}

		if (server)
			result = initServerSocket(&connectionData);

		else
			result = initClientSocket(&connectionData);


		if (result == NET_NO_ERROR)
			displayLocalIP();

		freeaddrinfo(reinterpret_cast<addrinfo*>(connectionData.m_addrInfo));

		return result;
	}


}