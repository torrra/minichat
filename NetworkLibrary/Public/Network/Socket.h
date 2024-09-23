#pragma once

#include <string>

#ifndef NET_DEFAULT_PORT
#define NET_DEFAULT_PORT 	"8888"
#endif

namespace net
{
	class Socket
	{
	public:
		using Handle_t = unsigned long long;

	private:

		using Port_t = unsigned short;

		enum SocketParams : Handle_t
		{
			INVALID_HANDLE = static_cast<Handle_t>(~0),
		};


	public:

					Socket(void) = default;
					Socket(Handle_t handle);
					~Socket(void);

		// Create a socket that will connect to a host IP and port
		int			createClient(const std::string& ipAddress,
								 const std::string& port);

		// Create a listening socket that will be bound to a port
		int			createServer(const std::string& port);

		// Disable communications
		int			shutdown(void)											const;

		// Close socket
		int			close(void)												const;

		// Accept incoming connection (for listening socket)
		Socket		accept(void)											const;

		// Create a WSA event that will be sent when the server is sending data to
		// a client socket
		void*		createServerEvent(void)									const;

		// Send to a specific socket
		int			sendTo(const Socket& socket,
						   const void* data, int size)						const;

		// Send to server this socket is connected to (client side)
		int			send(const void* data, int size)						const;

		// Receive from server this socket is connected to (client side)
		int			receive(void* buffer, int size)							const;

		// Receive from a specific socket
		int			receiveFrom(const Socket& socket,
								void* buffer, int size)						const;

		// Get socket handle
		Handle_t	getHandle()												const;

		// Socket handle is valid
		bool		isValid(void)											const;

		bool		operator==(const Socket& rhs)							const;
		bool		operator!=(const Socket& rhs)							const;

		// Server side pending connection queue
		static int& connectionBackLog();

	private:

		// Connect to host socket
		int			connect(void* addrData)									const;

		// Bind listening socket to port
		int			bind(void* addrData)									const;

		// Listen to incoming connections
		int			listen(void)											const;

		// Display this machine's local IP addresses
		void		displayLocalIP();

		// Convert service name or port number string to unsigned short
		Port_t      convertPortNumber(const std::string& portString);


		// Members

		Handle_t	m_handle = SocketParams::INVALID_HANDLE;


		// Static members

		static int  m_pendingConnectionCap;

	};
}
