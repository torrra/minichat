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

		enum SocketParams : Handle_t
		{
			INVALID_HANDLE = static_cast<Handle_t>(~0),
		};


	public:

					Socket(void) = default;
					Socket(Handle_t handle);

					~Socket(void);

		int			create(const std::string& hostname,
						   const std::string& port, bool server = false);


		Socket		accept(/*IPData& incomingData*/)						const;

		int			send(const Socket& socket,
						 const void* data, int size)						const;

		int			receive(void* data, int size)							const;

		Handle_t	getHandle()												const;

					operator bool(void)										const;

		static int& connectionBackLog();

	private:

		int			close(void)												const;
		int			connect(void* addrData)									const;

		int			bind(void* addrData)									const;
		int			listen(void)											const;

		int			createSocket(void* addrData);
		void		initSocketHints(void* addrData, bool server)			const;

		int			initClientSocket(void* addrData);
		int			initServerSocket(void* addrData);

		void		displayLocalIP();

		Handle_t	m_handle = SocketParams::INVALID_HANDLE;

		static int  m_pendingConnectionCap;

	};
}
