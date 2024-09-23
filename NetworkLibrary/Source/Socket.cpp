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
		int			result = 0;
		addrinfo*	addrList;
		IPData		socketData
		{
			.m_ipString = ipAddress.c_str(),
			.m_portString = port.c_str(),
		};

		// Create socket
		createDualStackSocket(&socketData);
		consoleOutput("Connecting...\n");
		addrList = static_cast<addrinfo*>(socketData.m_resultAddr);

		if (addrList)
			result = ::connect(m_handle, addrList->ai_addr, (int)addrList->ai_addrlen);

		if (result == NO_ERROR)
		{
			consoleOutput("Client connected successfully.\n\n");

			// enable nonblocking mode after connection
			enableNonBlocking();
		}
		else
			reportWindowsError("connect", WSAGetLastError());

		// free dynamically allocated addrinfo in createDualStackSocket()
		freeaddrinfo(addrList);
		return result;
	}

	int Socket::createServer(const std::string& port)
	{
		int			result = 0;
		addrinfo*	addrList;
		IPData		socketData
		{
			.m_portString = port.c_str()
		};

		// create socket supporting both IPV4 and IPV6
		createDualStackSocket(&socketData);
		addrList = static_cast<addrinfo*>(socketData.m_resultAddr);

		if (addrList)
			result = ::bind(m_handle, addrList->ai_addr, (int)addrList->ai_addrlen);

		if (result)
			reportWindowsError("bind", WSAGetLastError());

		this->listen();

		enableNonBlocking();
		displayLocalIP();

		freeaddrinfo(addrList);
		return result;
	}

	Socket Socket::accept() const
	{
		Handle_t			acceptedSocket;
		sockaddr_storage	incomingAddr;
		int					addrLength = sizeof incomingAddr;

		char				addressString[INET6_ADDRSTRLEN];
		unsigned long		bufferLength = INET6_ADDRSTRLEN;

		// Accept client socket
		acceptedSocket = ::accept(m_handle,
								  reinterpret_cast<sockaddr*>(&incomingAddr),
								  &addrLength);

		if (acceptedSocket == SocketParams::INVALID_HANDLE)
			reportWindowsError("Socket::accept", SOCKET_ERROR);

		else
		{
			addrLength = sizeof incomingAddr;

			// Get IP from incoming socket
			WSAAddressToStringA(reinterpret_cast<sockaddr*>(&incomingAddr),
								addrLength, nullptr, addressString, &bufferLength);

			consoleOutput("Connection received from: %1\n", addressString);

		}

		return Socket(acceptedSocket);
	}


	void* Socket::createServerEvent(void) const
	{
		// Create empty event
		void*	eventHandle = WSACreateEvent();

		if (!eventHandle)
			reportWindowsError("WSACreateEvent", WSAGetLastError());

		// Associate empty event to client sockets
		int		result = WSAEventSelect(m_handle, eventHandle, FD_READ);

		if (result == SOCKET_ERROR)
			reportWindowsError("WSAEventSelect", WSAGetLastError());

		return eventHandle;
	}

	int Socket::sendTo(const Socket& target, const void* data, int size) const
	{
		const int	noFlags = 0;

		// write received data into buffer
		// and return number of bytes received
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

		// write received data into buffer
		// and return number of bytes received
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

		// write received data into buffer
		// and return number of bytes received
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

		// write received data into buffer and return bytes received
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


	void Socket::displayLocalIP()
	{
		char			stringBuf[NI_MAXHOST];
		unsigned long	size = static_cast<int>(sizeof stringBuf);
		addrinfo*		result;
		int				error = gethostname(stringBuf, NI_MAXHOST);

		if (error != NO_ERROR)
			reportWindowsError("gethostname", WSAGetLastError());

		// Dynamically allocated
		 getaddrinfo(stringBuf, nullptr, nullptr, &result);

		if (error != NO_ERROR)
			reportWindowsError("getaddrinfo", error);

		// Display all IP addresses on this machine
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
		bool	digit = true;

		// Check if whole string is a number
		for (char character : portString)
		{
			if (!isdigit(character))
			{
				digit = false;
				break;
			}
		}

		// Simply convert digits to number
		if (digit)
			return static_cast<Port_t>(atoi(portString.c_str()));

		// Get port number from service name if string is not a number
		else
		{
			servent*	servEntry = getservbyname(portString.c_str(), nullptr);

			return ntohs(servEntry->s_port);
		}
	}

	int Socket::enableNonBlocking()
	{
		unsigned long	enabled = 1ul;

		// Disabling blocking mode will cause blocking send/receive calls to fail
		int				result = ioctlsocket(m_handle, FIONBIO, &enabled);

		if (result == SOCKET_ERROR)
			reportWindowsError("ioctlsocket", WSAGetLastError());

		return result;
	}

	int Socket::createDualStackSocket(void* ipData)
	{
		IPData*			socketData = static_cast<IPData*>(ipData);
		addrinfo		hints;
		addrinfo*		results;

		// Zero-out and assign hints
		memset(&hints, 0, sizeof addrinfo);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if (!socketData->m_ipString)
			hints.ai_flags = AI_PASSIVE;

		if(getaddrinfo(socketData->m_ipString, socketData->m_portString, &hints, &results))
			reportWindowsError("getaddrinfo", WSAGetLastError());

		// Create socket
		socketData->m_resultAddr = results;
		m_handle = ::socket(results->ai_family, results->ai_socktype, results->ai_protocol);

		if (m_handle == SocketParams::INVALID_HANDLE)
		{
			int		error = WSAGetLastError();

			reportWindowsError("::socket", error);
			return error;
		}

		return enableIPV4Support();
	}

	int Socket::enableIPV4Support()
	{
		unsigned long		enabled = 0ul;
		int					socketOptResult;

		// Set v6 only option to false
		socketOptResult	= setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY,
									(const char*)&enabled, sizeof enabled);

		if (socketOptResult != NO_ERROR)
			reportWindowsError("setsockopt", WSAGetLastError());

		return socketOptResult;

	}
}