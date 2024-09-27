#include "Chatroom.h"

namespace server
{
	Chatroom::Chatroom(const std::string& name, net::Socket::Handle_t user)
		: m_roomName(name)
	{
		m_users.push_back(user);
	}

	Chatroom::Chatroom(const std::string& name)
		: m_roomName(name)
	{
	}
}
