#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Socket.h"
#include "IPData.h"
#include "ErrorHandling.h"

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream> // TODO:error handling
#include <string.h>


namespace net
{
	int  Socket::m_pendingConnectionCap = 5;

	Socket::Socket(Handle_t handle)
		: m_handle(handle)
	{
	}

	Socket::~Socket(void)
	{
		this->close();
	}

	int Socket::create(const std::string& hostname,
		const std::string& port, bool server)
	{

		reportWSAError("Socket::create", 1234);

		int			result;
		IPData		connectionData
		{
			.m_ipString = hostname,
			.m_ipAddress = inet_addr(hostname.c_str()),
			//.m_ipVersion = this->m_ipVersion
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

	Socket Socket::accept() const
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
			reportWSAError("Socket::accept", SOCKET_ERROR);

		else
		{
			WSAAddressToString(reinterpret_cast<sockaddr*>(&incomingAddr),
				addrLength, nullptr, addressString, &bufferLength);

			consoleOutput("Connection received from: %1\n", addressString);

		}

		return Socket(acceptedSocket);

	}

	int Socket::send(const Socket& target, const void* data, int size) const
	{
		const int	noFlags = 0;

		int			result = ::send(target.m_handle,
			reinterpret_cast<const char*>(data),
			size, noFlags);

		if (result == SOCKET_ERROR)
			reportWSAError("::send", WSAGetLastError());

		return result;
	}

	int Socket::receive(void* buffer, int size) const
	{
		const int	noFlags = 0;

		int			bytesRecv = ::recv(m_handle,
									   reinterpret_cast<char*>(buffer),
									   size, noFlags);

		if (bytesRecv == SOCKET_ERROR)
			reportWSAError("::recv", WSAGetLastError());

		return bytesRecv;
	}

	Socket::Handle_t Socket::getHandle() const
	{
		return m_handle;
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
			reportWSAError("::close", WSAGetLastError());

		return result;
	}

	int Socket::listen(void) const
	{
		int		result = ::listen(m_handle, m_pendingConnectionCap);

		if (result != NET_NO_ERROR)
			reportWSAError("::listen", WSAGetLastError());

		return result;
	}

	int Socket::bind(void* addrData) const
	{
		int			result;
		sockaddr*	server = static_cast<sockaddr*>(addrData);

		result = ::bind(m_handle, server, sizeof * server);

		if (result != NET_NO_ERROR)
		{
			reportWSAError("::bind", WSAGetLastError());
			this->close();
			result = NET_WSA_BIND_ERROR;
		}

		return result;
	}

	int Socket::createSocket(void* addrData)
	{
		IPData*		ipData = static_cast<IPData*>(addrData);
		addrinfo*	addrPointer = static_cast<addrinfo*>(ipData->m_addrInfo);

		if (!addrPointer)
			return NET_WSA_SOCKET_ERROR;

		for (addrinfo* ptr = addrPointer; ptr != nullptr; ptr = ptr->ai_next)
		{
			m_handle = ::socket(ptr->ai_family,
								ptr->ai_socktype,
								ptr->ai_protocol);

			if (m_handle == SocketParams::INVALID_HANDLE)
			{
				reportWSAError("::socket", WSAGetLastError());;
				continue;
			}

			// Stop at first successful socket created
			break;
		}

		return NET_NO_ERROR;
	}

	void Socket::initSocketHints(void* addrData, bool server) const
	{
		addrinfo* hints = static_cast<addrinfo*>(addrData);

		if (!hints)
		{
			std::cerr << "Error initializing socket hints\n";
			return;
		}

		memset(hints, 0, sizeof * hints);

		hints->ai_family = AF_INET;
		hints->ai_socktype = SOCK_STREAM;
		hints->ai_protocol = IPPROTO_TCP;

		if (server)
			hints->ai_flags = AI_PASSIVE;
	}

	int Socket::initClientSocket(void* addrData)
	{

		IPData*			ipData = static_cast<IPData*>(addrData);
		sockaddr_in		server;
		addrinfo		socketHints;

		initSocketHints(&socketHints, true);

		int		result = getaddrinfo(ipData->m_ipString.c_str(), NULL,
			&socketHints, (addrinfo**)(&ipData->m_addrInfo));

		if (result != NET_NO_ERROR)
			return NET_WSA_SOCKET_ERROR;

		result = createSocket(ipData);

		if (result != NET_NO_ERROR)
			__debugbreak();

		server.sin_addr.s_addr = ipData->m_ipAddress;
		server.sin_family = AF_INET;
		server.sin_port = htons(ipData->m_portNumber);

		return this->connect(&server);
	}

	int Socket::initServerSocket(void* addrData)
	{
		IPData*		ipData = static_cast<IPData*>(addrData);
		addrinfo	socketHints;

		initSocketHints(&socketHints, true);

		int		result = getaddrinfo(ipData->m_ipString.c_str(), NULL,
			&socketHints, (addrinfo**)(&ipData->m_addrInfo));

		if (result != NET_NO_ERROR)
			return NET_WSA_SOCKET_ERROR;

		result = createSocket(ipData);

		sockaddr_in		server;

		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_family = AF_INET;
		server.sin_port = htons(ipData->m_portNumber);
		//server.

		result = this->bind(&server);

		this->listen();

		return result;
	}

	void Socket::displayLocalIP()
	{
		char		stringBuf[INET6_ADDRSTRLEN];
		char* ip;
		hostent* hostEntry;

		gethostname(stringBuf, INET6_ADDRSTRLEN);
		hostEntry = gethostbyname(stringBuf);
		ip = inet_ntoa(*(reinterpret_cast<in_addr*>(hostEntry->h_addr_list[0])));


		std::cout << "Local address: " << ip << '\n';
	}

	int Socket::connect(void* addrData) const
	{
		int				result;
		sockaddr_in* server = static_cast<sockaddr_in*>(addrData);

		result = ::connect(m_handle, (sockaddr*)server, sizeof * server);

		if (result != NET_NO_ERROR)
		{
			std::cerr << "Error: connect: " << WSAGetLastError() << "\n";
			this->close();
			result = NET_WSA_CONNECT_ERROR;
		}

		return result;
	}
}