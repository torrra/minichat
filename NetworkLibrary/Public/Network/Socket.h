#pragma once

#include <string>

#include "Constants.h"
//#include "IPData.h"

namespace net
{
	// Size = 16B, padding = 7B

	class Socket
	{
	private:

		using Handle_t = unsigned long long;
		using Port_t = unsigned short;

		enum SocketParams : Handle_t
		{
			INVALID_HANDLE = static_cast<Handle_t>(~0),
		};


	public:

					Socket(void) = default;
					Socket(Handle_t handle);

					~Socket(void);

		int			createClient(const std::string& ipAddress,
								 const std::string& port);

		int			createServer(const std::string& port);

		int			shutdown(void)											const;
		int			close(void)												const;

		Socket		accept(void)											const;

		int			sendTo(const Socket& socket,
						   const void* data, int size)						const;

		int			send(const void* data, int size)						const;

		int			receive(void* buffer, int size)							const;

		int			receiveFrom(const Socket& socket,
								void* buffer, int size)						const;

		Handle_t	getHandle()												const;

					operator bool(void)										const;

		static int& connectionBackLog();

	private:

		int			create(const std::string& hostname,
						   const std::string& port, bool server = false);

		int			connect(void* addrData)									const;

		int			bind(void* addrData)									const;
		int			listen(void)											const;

		int			createSocket(void* addrData);
		void		initSocketHints(void* addrData, bool server)			const;

		int			initClientSocket(void* addrData);
		int			initServerSocket(void* addrData);

		void		displayLocalIP();

		Port_t      convertPortNumber(const std::string& portString);

		Handle_t	m_handle = SocketParams::INVALID_HANDLE;

		static int  m_pendingConnectionCap;

	};
}
