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


}