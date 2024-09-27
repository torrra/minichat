#pragma once

#include <vector>
#include <string>

#include <Network/Socket.h>
#include <Network/Packet.h>

namespace server
{
	struct Chatroom
	{
		Chatroom(const std::string& name, net::Socket::Handle_t user);
		~Chatroom(void) = default;

		std::string						m_roomName;
		std::vector<net::Socket>		m_users;
	};
}