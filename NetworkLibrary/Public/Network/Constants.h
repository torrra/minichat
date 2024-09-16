#pragma once

namespace net
{
	enum NetResult : int
	{
		NET_NO_ERROR          =	0,

		NET_WSA_STARTUP_ERROR =	1'01,
		NET_WSA_CLEANUP_ERROR =	1'02,

		NET_WSA_SOCKET_ERROR  =	1'03,

		NET_WSA_BIND_ERROR	  = 1'04,
		NET_WSA_LISTEN_ERROR  =	1'05,

		NET_WSA_CONNECT_ERROR =	1'06
	};

	enum class IPVersion : unsigned char
	{
		IP_V4,
		IP_V6
	};
}