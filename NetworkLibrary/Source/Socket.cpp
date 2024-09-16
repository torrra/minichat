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

	Socket Socket::accept(/*IPData& incomingData*/) const
	{
		Handle_t			acceptedSocket;
		sockaddr_storage	incomingAddr;

		char				addressString[INET6_ADDRSTRLEN];
		unsigned long		bufferLength = INET6_ADDRSTRLEN;

		int					addrLength = sizeof incomingAddr;

		acceptedSocket = ::accept(m_handle,
			reinterpret_cast<sockaddr*>(&incomingAddr),
			&addrLength);

		if (acceptedSocket == SocketParams::INVALID_HANDLE)
			std::cerr << "Error: accept\n";

		else
		{
			WSAAddressToString(reinterpret_cast<sockaddr*>(&incomingAddr),
				addrLength, nullptr, addressString, &bufferLength);

			std::cout << "Connection received from: " << addressString << "\n";

		}

		return Socket(acceptedSocket, m_ipVersion);

	}

	int Socket::send(const Socket& target, const void* data, int size) const
	{
		const int	noFlags = 0;

		int			result = ::send(target.m_handle,
			reinterpret_cast<const char*>(data),
			size, noFlags);

		if (result != NET_NO_ERROR)
			std::cerr << "Error: send " << WSAGetLastError() << "\n";

		return result;
	}

	int Socket::receive(void* buffer, int size) const
	{
		const int	noFlags = 0;

		int			bytesRecv = ::recv(m_handle,
			reinterpret_cast<char*>(buffer),
			size, noFlags);

		if (!bytesRecv)
			std::cerr << "Error: no bytes received\n";

		else if (bytesRecv == SOCKET_ERROR)
			std::cerr << "Error: socket error on receive\n";

		return bytesRecv;
	}


	Socket::Handle_t Socket::getHandle() const
	{
		return m_handle;
	}

	IPVersion Socket::getIPVersion() const
	{
		return m_ipVersion;
	}

	Socket::operator bool(void) const
	{
		return m_handle != SocketParams::INVALID_HANDLE;
	}

	int& Socket::connectionBackLog()
	{
		return m_pendingConnectionCap;
	}


	int Socket::close() const
	{
		int		result = ::closesocket(m_handle);

		if (result != NET_NO_ERROR)
			std::cerr << "Error: close\n";

		return result;
	}

	int Socket::listen(void) const
	{
		int		result = ::listen(m_handle, m_pendingConnectionCap);

		if (result != NET_NO_ERROR)
			std::cerr << "Error: listen\n";

		return result;
	}

	int Socket::bind(void* addrData) const
	{
		int			result;
		sockaddr* server = static_cast<sockaddr*>(addrData);

		result = ::bind(m_handle, server, sizeof * server);

		if (result != NET_NO_ERROR)
		{
			std::cerr << "Error: bind " << WSAGetLastError() << "\n";
			this->close();
			result = NET_WSA_BIND_ERROR;
		}

		return result;
	}



}