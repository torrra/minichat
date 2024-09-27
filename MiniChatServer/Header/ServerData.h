#pragma once

#include <Network/Server.h>
#include <unordered_map>

#include "Chatroom.h"

namespace server
{
    struct ServerData
	{
		using ClientMap = std::unordered_map<net::Socket::Handle_t, std::string>;
		using RoomMap = std::unordered_map<net::Socket::Handle_t, size_t>;

		net::Server					m_server;
		ClientMap					m_clientNames;
		RoomMap						m_clientRooms;
		std::vector<Chatroom>		m_rooms;
	};

	class ServerApp
	{
	private:

		struct Message
		{
			std::string				m_text;
			net::Socket::Handle_t	m_sender;
		};

		using ClientMap = std::unordered_map<net::Socket::Handle_t, std::string>;
		using RoomMap = std::unordered_map<net::Socket::Handle_t, size_t>;

	public:

		ServerApp(void) = default;
		~ServerApp(void) = default;

		// Receive and read all messages
		void			updateServer(void);
		void			processMessages();

		// Accessors

		bool			isRunning()				const;
		net::Server&	server();

	private:

		// Command handling

		bool	checkCommands(void);
		void	nicknameCommand(void);
		void	shutdownCommand(void);

		// Username processing
		void    displayCurrentUsers(void);

		// Chatroom creation / assignment

		bool	createChatroom(void);
		void	moveToRoom(void);

		void    removeFromRoom(Chatroom& room, net::Socket toRemove,
							   bool disconnected = false);

		void    assignRoom(const net::Socket::Handle_t client);

		// Send message to correct room

		void    sendReceivedMessage(void);
		void	formatReceivedMessage(void);
		void	sendToServerRoom(Chatroom& room, const std::string& message);


		// Utility

		void	removeNewlines(std::string& string);

		net::Server					m_server;
		ClientMap					m_clientNames;
		RoomMap						m_clientRooms;
		Message						m_currentMessage;
		std::vector<Chatroom>		m_rooms;
		bool						m_running = true;
	};
}