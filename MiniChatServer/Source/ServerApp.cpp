#include "ServerApp.h"

#include <Network/ConsoleOutput.h>

#ifndef SHUTDOWN_MESSAGE
#define SHUTDOWN_MESSAGE    "jrII0JB0y6"
#endif // !SHUTDOWN_MESSAGE


namespace server
{
    void ServerApp::updateServer(void)
    {
        m_server.serverUpdate();

        const net::Server::SocketArray&     invalidClients = m_server.getInvalid();

        for (const net::Socket& disconnected : invalidClients)
        {
            // do not attempt to delete a user if no
            // room was created to store their data
            if (m_rooms.empty())
                break;

            size_t          roomID = m_clientRooms[disconnected.getHandle()];
            Chatroom&       room = m_rooms[roomID];
            std::string     leaveMsg = m_clientNames[disconnected.getHandle()];

            leaveMsg += " has left\r\n\r\n";

            // delete all information about this user and alert other
            // users in the same room
            removeFromRoom(room, disconnected, true);
            sendToServerRoom(room, leaveMsg);
        }
        processMessages();
    }

    void ServerApp::processMessages(void)
    {
        std::vector<net::Packet>    packets = m_server.receiveAllPackets();

        for (const net::Packet& received : packets)
        {
            m_currentMessage =
            {
                .m_text = net::Packet::unpackMessage(received),
                .m_sender = received.getSender().getHandle()
            };

            // assign default room if necessary
            assignRoom(m_currentMessage.m_sender);

            // send message to other clients if it is not a command
            if (!checkCommands())
                formatReceivedMessage();
        }

        // send packets that should
        // be sent to all clients
        m_server.sendAllPackets();
    }

    bool ServerApp::isRunning() const
    {
        return m_running;
    }

    net::Server& ServerApp::server()
    {
        return m_server;
    }

    bool ServerApp::checkCommands(void)
    {
        if (m_currentMessage.m_text[0] != '/')
            return false;

        // check command type
        switch (m_currentMessage.m_text[1])
        {
        case 's': shutdownCommand(); return true;

        case 'n': nicknameCommand(); return true;

        case 'c': createChatroom(); return true;

        case 'm': moveToRoom(); return true;

        case 'l': displayCurrentUsers(); return true;
        default: break;
        }

        return false;
    }

    void ServerApp::shutdownCommand(void)
    {
        if (m_currentMessage.m_text.find("shutdown") != std::string::npos)
        {
            size_t      len = strnlen_s(SHUTDOWN_MESSAGE, NET_MAX_PACKET_SIZE);

            m_running = false;
            net::consoleOutput("Shutting down server...\n");

            // send shutdown message to all clients
           m_server.createPacket(SHUTDOWN_MESSAGE, len);
        }
    }

    void ServerApp::nicknameCommand(void)
    {
        size_t      pos = m_currentMessage.m_text.find(' ');

        // invalid command or error reading "register" string
        if (pos == std::string::npos)
            return;

        bool            isNew = !m_clientNames.contains(m_currentMessage.m_sender);
        std::string     username = m_currentMessage.m_text.substr(pos + 1ull);

        if (!isNew)
        {
            // remove newline characters that may have been added by client
            removeNewlines(username);

            size_t           roomID = m_clientRooms[m_currentMessage.m_sender];
            Chatroom&        userRoom = m_rooms[roomID];
            std::string      renameMsg = m_clientNames[m_currentMessage.m_sender] +
                                        " has been renamed to " + username +
                                        "\r\n\r\n";

            // send name update to all users
            sendToServerRoom(userRoom, renameMsg);
            m_clientNames[m_currentMessage.m_sender] = username;
        }
        else
        {
           // list other users connected if client is new
           m_clientNames[m_currentMessage.m_sender] = username;
           sendToServerRoom(m_rooms[0], username + " has joined.\r\n\r\n");
           displayCurrentUsers();
        }
    }

    void ServerApp::displayCurrentUsers(void)
    {
        net::Socket             severSocket = m_server.getSocket();
        std::string             username = m_clientNames[m_currentMessage.m_sender];
        std::string             connectedUsers = "Other users connected: ";
        net::Socket::Handle_t   senderHandle = m_currentMessage.m_sender;

        // add all clients to current users list
        for (auto& client :m_clientNames)
        {
            if (client.first == senderHandle ||
                m_clientRooms[senderHandle] != m_clientRooms[client.first])
            {
                continue;
            }

            connectedUsers += client.second;
            connectedUsers += ", ";
        }
        connectedUsers += "\r\n";

        // send message to new user only
        severSocket.sendTo(m_currentMessage.m_sender,
                           connectedUsers.c_str(),
                           (int)connectedUsers.size());

    }

    void ServerApp::removeNewlines(std::string& str)
    {
        for (size_t index = str.size() - 1ull; index > 0ull; --index)
        {
            // delete newline characters and leave at least
            // 1 character in string
            switch (str[index])
            {
            case '\n':
                str.pop_back();
                continue;

            case '\r':
                str.pop_back();
                continue;

            default: return;
            }
        }
    }

    bool ServerApp::createChatroom(void)
    {
        size_t          pos = m_currentMessage.m_text.find('#');

        if (pos == std::string::npos)
            return false;

        std::string     roomName = m_currentMessage.m_text.substr(pos);
        size_t          currentIndex = m_clientRooms[m_currentMessage.m_sender];
        Chatroom&       currentRoom = m_rooms[currentIndex];

        if (roomName.empty())
            return false;

        removeNewlines(roomName);

        // remove user from old room and add to newly created channel
        removeFromRoom(currentRoom, m_currentMessage.m_sender);
        m_clientRooms[m_currentMessage.m_sender] = m_rooms.size();
        m_rooms.emplace_back(roomName, m_currentMessage.m_sender);

        std::string     roomMsg = "Channel " + roomName + " was created.\r\n\r\n";
        std::string     moveMsg = "Moved to channel " + roomName + ".\r\n";
        net::Socket     serverSock = m_server.getSocket();

        // tell users that a new room was created
        net::consoleOutput(roomMsg.c_str());
        m_server.createPacket(roomMsg.c_str(), roomMsg.size());

        m_server.getSocket().sendTo(m_currentMessage.m_sender, moveMsg.c_str(),
                                    (int) moveMsg.size());
        return true;
    }

    void ServerApp::moveToRoom(void)
    {
        std::string     msgCopy = m_currentMessage.m_text;
        size_t          roomIndex = 0;
        size_t          namePos = msgCopy.find('#');

        if (namePos == std::string::npos)
            return;

        removeNewlines(msgCopy);
        msgCopy = msgCopy.substr(namePos);

        for (Chatroom& room : m_rooms)
        {
            // wrong room name
            if (room.m_roomName.compare(msgCopy) != 0)
            {
                ++roomIndex;
                continue;
            }
            room.m_users.push_back(m_currentMessage.m_sender);

            std::string     username = m_clientNames[m_currentMessage.m_sender];
            size_t          currentIndex = m_clientRooms[m_currentMessage.m_sender];
            Chatroom&       currentRoom = m_rooms[currentIndex];

            removeFromRoom(currentRoom, m_currentMessage.m_sender);
            m_clientRooms[m_currentMessage.m_sender] = roomIndex;

            // tell users from old room that this user has left, and alert
            // clients from new room that they have joined
            sendToServerRoom(room, username + " has joined.\r\n\r\n");
            sendToServerRoom(currentRoom, username + " has left.\r\n\r\n");
            displayCurrentUsers();
            break;
        }
    }

    void ServerApp::sendReceivedMessage(void)
    {
        size_t          roomID = m_clientRooms[m_currentMessage.m_sender];
        Chatroom&       room = m_rooms[roomID];
        net::Socket     serverSocket = m_server.getSocket();

        // send message from user to all other users in a room
        for (net::Socket& recipient : room.m_users)
        {
            if (recipient == m_currentMessage.m_sender)
                continue;

            serverSocket.sendTo(recipient, m_currentMessage.m_text.c_str(),
                               (int)m_currentMessage.m_text.size());
        }
    }

    void ServerApp::formatReceivedMessage(void)
    {
        std::string     toDisplay;
        size_t          roomIndex = m_clientRooms[m_currentMessage.m_sender];

        toDisplay = m_rooms[roomIndex].m_roomName;

        // add username before message
        toDisplay += " | " +m_clientNames[m_currentMessage.m_sender];
        toDisplay += " > " + m_currentMessage.m_text;
        net::consoleOutput(toDisplay.c_str());

        m_currentMessage.m_text = toDisplay;
        sendReceivedMessage();
    }

    void ServerApp::sendToServerRoom(Chatroom& room, const std::string& message)
    {
        // send message to all users in a given room
        for (const net::Socket& client : room.m_users)
        {
            m_server.getSocket().sendTo(client, message.c_str(),
                                        (int) message.size());
        }
    }

    void ServerApp::removeFromRoom(Chatroom& room, net::Socket toRemove,
                                   bool disconnected)
    {
        // find user and remove from room user list
        for (auto iterator = room.m_users.begin();
            iterator != room.m_users.end(); ++iterator)
        {
            if (*iterator != toRemove)
                continue;

            // remove user from all records if connection
            // was lost or terminated
            if(disconnected)
            {
                m_clientRooms.erase(toRemove.getHandle());
                m_clientNames.erase(toRemove.getHandle());
            }

            room.m_users.erase(iterator);
            break;

        }
    }

    void ServerApp::assignRoom(const net::Socket::Handle_t client)
    {
        // Create default room if none already exists
        if (m_rooms.empty())
           m_rooms.emplace_back("#general");

        // assign new user to #general by default
        if (!m_clientRooms.contains(client))
        {
           m_clientRooms[client] = 0;
           m_rooms[0].m_users.push_back(client);
        }
    }

}
