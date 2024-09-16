#pragma once

#include <string>

#include "Constants.h"

namespace net
{
	// size: 56B, padding : 2B

	// Contains information about an IP endpoint
	struct IPData
	{
		std::string		m_ipString;

		// WinSock addrinfo struct
		void*			m_addrInfo;

		// IPV4 address as 4B unsigned integer
		unsigned long	m_ipAddress;

		// Port number (host byte order)
		unsigned short	m_portNumber;
		//IPVersion		m_ipVersion;
	};
}