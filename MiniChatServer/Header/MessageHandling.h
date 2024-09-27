#pragma once

#include <Network/Server.h>
#include <unordered_map>

#include "ServerData.h"

namespace server
{
	struct Message
	{
		std::string				m_text;
		net::Socket::Handle_t	m_sender;
	};

	// Receive and read all messages
	void	processMessages(ServerData& server, bool& running);

	// Command handling

	bool	checkCommands(const Message& msg, ServerData& server, bool& running);
	void	sysSubCommands(const Message& msg, ServerData& server, bool& running);
	void	userSubCommands(const Message& msg, ServerData& server);

	// Username processing
	void    displayCurrentUsers(const Message& msg, ServerData& server);

	// chatrooms
	bool	createChatroom(const Message& msg, ServerData& server);
	void	moveToRoom(const Message& msg, ServerData& server);
	void    sendToRoom(const Message& msg, ServerData& server);
	void	findSenderRoom(Message& msg, ServerData& server);

	void    removeFromRoom(Chatroom& room, const net::Socket& toRemove);
	void    assignRoom(const net::Socket::Handle_t client, ServerData& server);

	// Utility

	void	removeNewlines(std::string& string);
}