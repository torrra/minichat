#pragma once

#include <Network/Server.h>

#include <unordered_map>

namespace server
{
    struct ServerData
	{
		using ClientMap = std::unordered_map<net::Socket::Handle_t, std::string>;

		ClientMap		m_clientNames;
		net::Server		m_server;
	};
}