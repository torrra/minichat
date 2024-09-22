#include "Socket.h"
#include "ErrorHandling.h"
#include "ConsoleOutput.h"

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Ws2tcpip.h>
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
	}

	int Socket::createClient(const std::string& ipAddress, const std::string& port)
	{
		m_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_handle == SocketParams::INVALID_HANDLE)
		{
			int		error = WSAGetLastError();
			reportWindowsError("::socket", error);

			return error;
		}

		// server to connect to
		sockaddr_in		server;
		int				result;
		unsigned long	optionEnabled = 1ul;

		// what address and port this client will connect to
		server.sin_addr.s_addr = inet_addr(ipAddress.c_str());
		server.sin_family = AF_INET;
		server.sin_port = htons(convertPortNumber(port));

		consoleOutput("Connecting...\n");
		result = this->connect(&server);

		if (result == NO_ERROR)
		{
			consoleOutput("Client connected successfully.\n\n");

			// enable nonblocking mode after connection
			result = ioctlsocket(m_handle, FIONBIO, &optionEnabled);

			if (result == SOCKET_ERROR)
				reportWindowsError("ioctlsocket", WSAGetLastError());
		}

		return result;
	}

	int Socket::createServer(const std::string& port)
	{
		m_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_handle == SocketParams::INVALID_HANDLE)
		{
			int		error = WSAGetLastError();
			reportWindowsError("::socket", error);

			return error;
		}

		int				result;
		sockaddr_in		server;
		unsigned long	optionEnabled = 0ul;

		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_family = AF_INET;
		server.sin_port = htons(convertPortNumber(port));

		result = setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY,
							(const char*)&optionEnabled, sizeof optionEnabled);

		result = this->bind(&server);

		if (result)
			return result;

		result = this->listen();
		optionEnabled = 1ul;
		result = ioctlsocket(m_handle, FIONBIO, &optionEnabled);

		if (result == SOCKET_ERROR)
			reportWindowsError("ioctlsocket", WSAGetLastError());

		displayLocalIP();
		return result;

	}


	int Socket::create(const std::string& hostname,
		const std::string& port, bool server)
	{
		int			result;
		IPData		connectionData
		{
			.m_ipString = hostname,
			.m_ipAddress = inet_addr(hostname.c_str())
		};

		if (isdigit(port[0]))
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


		if (result == NO_ERROR)
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
			reportWindowsError("Socket::accept", SOCKET_ERROR);

		else
		{
			addrLength = sizeof incomingAddr;

			WSAAddressToStringA(reinterpret_cast<sockaddr*>(&incomingAddr),
				addrLength, nullptr, addressString, &bufferLength);

			consoleOutput("Connection received from: %1\n", addressString);

		}

		return Socket(acceptedSocket);

	}


	void* Socket::createServerEvent(void) const
	{
		void*	eventHandle = WSACreateEvent();

		if (!eventHandle)
			reportWindowsError("WSACreateEvent", WSAGetLastError());

		int		result = WSAEventSelect(m_handle, eventHandle, FD_READ);

		if (result == SOCKET_ERROR)
			reportWindowsError("WSAEventSelect", WSAGetLastError());

		return eventHandle;
	}

	int Socket::sendTo(const Socket& target, const void* data, int size) const
	{
		const int	noFlags = 0;

		int			result = ::send(target.m_handle,
			reinterpret_cast<const char*>(data),
			size, noFlags);

		if (result == SOCKET_ERROR)
			reportWindowsError("::send", WSAGetLastError());

		return result;
	}

	int Socket::send(const void* data, int size) const
	{
		const int	noFlags = 0;

		int			result = ::send(m_handle,
			reinterpret_cast<const char*>(data),
			size, noFlags);

		if (result == SOCKET_ERROR)
			reportWindowsError("::send", WSAGetLastError());

		return result;
	}

	int Socket::receive(void* buffer, int size) const
	{
		const int	noFlags = 0;

		int			bytesRecv = ::recv(m_handle,
									   reinterpret_cast<char*>(buffer),
									   size, noFlags);

		if (bytesRecv == SOCKET_ERROR)
			reportWindowsError("::recv", WSAGetLastError());

		return bytesRecv;
	}

	int Socket::receiveFrom(const Socket& socket, void* buffer, int size) const
	{
		const int	noFlags = 0;

		int			bytesRecv = ::recv(socket.m_handle,
									   reinterpret_cast<char*>(buffer),
									   size, noFlags);

		if (bytesRecv == SOCKET_ERROR)
			reportWindowsError("::recv", WSAGetLastError());

		return bytesRecv;
	}

	Socket::Handle_t Socket::getHandle() const
	{
		return m_handle;
	}


	bool Socket::isValid(void) const
	{
		return m_handle != SocketParams::INVALID_HANDLE;
	}

	bool Socket::operator==(const Socket& rhs) const
	{
		return m_handle == rhs.m_handle;
	}

	bool Socket::operator!=(const Socket& rhs) const
	{
		return !(*this == rhs);
	}

	int& Socket::connectionBackLog()
	{
		return m_pendingConnectionCap;
	}


	int Socket::shutdown(void) const
	{
		int result = ::shutdown(m_handle, SD_BOTH);

		if (result != NO_ERROR)
			reportWindowsError("::shutdown", WSAGetLastError());

		return result;
	}

	int Socket::close() const
	{
		int		result = ::closesocket(m_handle);

		if (result != NO_ERROR)
			reportWindowsError("::close", WSAGetLastError());

		return result;
	}

	int Socket::listen(void) const
	{
		int		result = ::listen(m_handle, SOMAXCONN);

		if (result != NO_ERROR)
			reportWindowsError("::listen", WSAGetLastError());

		return result;
	}

	int Socket::bind(void* addrData) const
	{
		int			result;
		sockaddr*	server = static_cast<sockaddr*>(addrData);

		result = ::bind(m_handle, server, sizeof *server);

		if (result != NO_ERROR)
		{
			reportWindowsError("::bind", WSAGetLastError());
			this->close();
		}

		return result;
	}

	int Socket::createSocket(void* addrData)
	{
		IPData*		ipData = static_cast<IPData*>(addrData);
		addrinfo*	addrPointer = static_cast<addrinfo*>(ipData->m_addrInfo);

		if (!addrPointer)
		{
			consoleOutput("createSocket: bad pointer\n");
			return SOCKET_ERROR;
		}

		for (addrinfo* ptr = addrPointer; ptr != nullptr; ptr = ptr->ai_next)
		{
			m_handle = ::socket(ptr->ai_family,
								ptr->ai_socktype,
								ptr->ai_protocol);

			if (m_handle == SocketParams::INVALID_HANDLE)
			{
				reportWindowsError("::socket", WSAGetLastError());;
				continue;
			}

			// Stop at first successful socket created
			break;
		}

		return NO_ERROR;
	}

	void Socket::initSocketHints(void* addrData, bool server) const
	{
		addrinfo* hints = static_cast<addrinfo*>(addrData);

		if (!hints)
		{
			consoleOutput("initSocketHints: invalid hint pointer");
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
									&socketHints,
									(addrinfo**)(&ipData->m_addrInfo));

		if (result != NO_ERROR)
			return result;



		result = createSocket(ipData);

		if (result != NO_ERROR)
			DebugBreak();

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
									&socketHints,
									(addrinfo**)(&ipData->m_addrInfo));

		if (result != NO_ERROR)
			return result;

		result = createSocket(ipData);

		sockaddr_in		server;

		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_family = AF_INET;
		server.sin_port = htons(ipData->m_portNumber);

		result = this->bind(&server);

		this->listen();

		return result;
	}

	void Socket::displayLocalIP()
	{
		char		stringBuf[NI_MAXHOST];
		addrinfo*	result;

		int error = gethostname(stringBuf, NI_MAXHOST);

		if (error != NO_ERROR)
			reportWindowsError("gethostname", WSAGetLastError());

		// Dynamically allocated
		 getaddrinfo(stringBuf, nullptr, nullptr, &result);

		if (error != NO_ERROR)
			reportWindowsError("getaddrinfo", error);


		unsigned long size = static_cast<int>(sizeof stringBuf);

		for (addrinfo* pointer = result; pointer != nullptr; pointer = pointer->ai_next)
		{
			error = WSAAddressToStringA(pointer->ai_addr,
										(unsigned int) pointer->ai_addrlen,
										nullptr, stringBuf, &size);

			if (error != NO_ERROR)
			{
				reportWindowsError("WSAAddressToString", WSAGetLastError());
				__debugbreak();
			}

			consoleOutput("Local address: %1\n", stringBuf);
			size = static_cast<int>(sizeof stringBuf);
		}

		consoleOutput("Socket handle: %1!llu!\n", m_handle);
		freeaddrinfo(result);
	}

	Socket::Port_t Socket::convertPortNumber(const std::string& portString)
	{
		if (isdigit(portString[0]))
			return static_cast<Port_t>(atoi(portString.c_str()));

		else
		{
			servent* servEntry = getservbyname(portString.c_str(), nullptr);

			return ntohs(servEntry->s_port);
		}
	}



	int Socket::connect(void* addrData) const
	{
		int				result;
		sockaddr_in*	server = static_cast<sockaddr_in*>(addrData);

		result = ::connect(m_handle, (sockaddr*)server, sizeof * server);

		if (result != NO_ERROR)
		{
			reportWindowsError("::connect", WSAGetLastError());
			this->close();
		}

		return result;
	}
}