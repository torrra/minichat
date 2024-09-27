#include <Network/ConsoleOutput.h>

#include "MessageHandling.h"

#ifndef SHUTDOWN_MSG
#define SHUTDOWN_MSG    "jrII0JB0y6"
#endif // !NET_SHUTDOWN_MSG


namespace server
{
    void processMessages(ServerData& data, bool& running)
    {
        std::vector<net::Packet>    packets = data.m_server.receiveAllPackets();

        for (const net::Packet& received : packets)
        {
            Message     message =
            {
                .m_text = net::Packet::unpackMessage(received),
                .m_sender = received.getSender().getHandle()
            };

            assignRoom(message.m_sender, data);

            // send message to other clients if it is not a command
            if (!checkCommands(message, data, running))
                findSenderRoom(message, data);
        }
        data.m_server.sendAllPackets();
    }

    bool checkCommands(const Message& msg, ServerData& server, bool& running)
    {
        if (msg.m_text[0] != '/')
            return false;

        // check command type
        switch (msg.m_text[1])
        {

        // system commands
        case 's':
            sysSubCommands(msg, server, running);
            return true;

        // commands related to user data
        case 'u':
            userSubCommands(msg, server);
            return true;

        case 'c':
            createChatroom(msg, server);
            return true;

        case 'm':
            moveToRoom(msg, server);
            return true;

        default: break;
        }

        return false;
    }

    void sysSubCommands(const Message& msg, ServerData& server, bool& running)
    {
        if (msg.m_text.find("shutdown") != std::string::npos)
        {
            size_t      msgLen = strnlen_s(SHUTDOWN_MSG, NET_MAX_PACKET_SIZE);

            running = false;
            net::consoleOutput("Shutting down server...\n");

            // send shutdown message to all clients
            server.m_server.createPacket(SHUTDOWN_MSG, msgLen);
        }
    }

    void userSubCommands(const Message& msg, ServerData& server)
    {
        size_t      pos = msg.m_text.find("register");
        size_t      len = strnlen_s("register", NET_MAX_PACKET_SIZE);

        // invalid command or error reading "register" string
        if (pos == std::string::npos || len >= msg.m_text.size())
            return;

        bool            isNew = !server.m_clientNames.contains(msg.m_sender);
        std::string     username = msg.m_text.substr(pos + len + 1ull);

        if (!isNew)
        {
            // remove newline characters that may have been added by client
            removeNewlines(username);

            std::string renameMsg = server.m_clientNames[msg.m_sender] +
                                   " has been renamed to " + username + "\r\n\r\n";

            // send name update to all users
            server.m_server.createPacket(renameMsg.c_str(), renameMsg.size());
            server.m_clientNames[msg.m_sender] = username;
        }
        else
        {
            // list other users connected if client is new
            server.m_clientNames[msg.m_sender] = username;
            displayCurrentUsers(msg, server);
        }
    }

    void displayCurrentUsers(const Message& msg, ServerData& server)
    {
        net::Socket     severSocket = server.m_server.getSocket();
        std::string     username = server.m_clientNames[msg.m_sender];
        std::string     connectedUsers = "Other users connected: ";

        // add all clients to current users list
        for (auto& client : server.m_clientNames)
        {
            if (client.first == msg.m_sender)
                continue;

            connectedUsers += client.second;
            connectedUsers += ", ";
        }
        connectedUsers += "\r\n";

        // send message to new user only
        severSocket.sendTo(msg.m_sender,
                           connectedUsers.c_str(),
                           (int)connectedUsers.size());

        username += " has joined.\r\n\r\n";

        // tell other clients that a new user has joined
        server.m_server.createPacket(username.c_str(),
                                     username.size(),
                                     msg.m_sender);
    }

    void removeNewlines(std::string& str)
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

    bool createChatroom(const Message& msg, ServerData& server)
    {
        size_t          pos = msg.m_text.find('#');
        std::string     roomName = msg.m_text.substr(pos);
        size_t          currentIndex = server.m_clientRooms[msg.m_sender];
        Chatroom&       currentRoom = server.m_rooms[currentIndex];

        if (roomName.empty())
            return false;

        removeNewlines(roomName);
        removeFromRoom(currentRoom, msg.m_sender);

        server.m_clientRooms[msg.m_sender] = server.m_rooms.size();
        server.m_rooms.emplace_back(roomName, msg.m_sender);

        std::string     roomMsg = "Room created successfully\r\n";
        net::Socket     serverSock = server.m_server.getSocket();

        serverSock.sendTo(msg.m_sender, roomMsg.c_str(), (int)roomMsg.size());
        net::consoleOutput(roomMsg.c_str());
        return true;
    }

    void moveToRoom(const Message& msg, ServerData& server)
    {
        std::string     msgCopy = msg.m_text;
        size_t          roomIndex = 0;
        size_t          namePos = msgCopy.find('#');

        if (namePos == std::string::npos)
            return;

        removeNewlines(msgCopy);
        msgCopy = msgCopy.substr(namePos);

        for (Chatroom& room : server.m_rooms)
        {
            if (room.m_roomName.compare(msgCopy) == 0)
            {
                room.m_users.push_back(msg.m_sender);

                std::string     message = server.m_clientNames[msg.m_sender];
                size_t          currentIndex = server.m_clientRooms[msg.m_sender];
                Chatroom&       currentRoom = server.m_rooms[currentIndex];

                removeFromRoom(currentRoom, msg.m_sender);
                server.m_clientRooms[msg.m_sender] = roomIndex;

                message += " has moved to room ";
                message += room.m_roomName + "\r\n\r\n";
                net::consoleOutput(message.c_str());

                server.m_server.createPacket(message.c_str(),
                                            (int) message.size());

                break;
            }

            ++roomIndex;
        }
    }

    void sendToRoom(const Message& msg, ServerData& server)
    {
        size_t          roomID = server.m_clientRooms[msg.m_sender];
        Chatroom&       room = server.m_rooms[roomID];
        net::Socket     serverSocket = server.m_server.getSocket();

        for (net::Socket& recipient : room.m_users)
        {
            if (recipient == msg.m_sender)
                continue;

            serverSocket.sendTo(recipient, msg.m_text.c_str(), (int)msg.m_text.size());
        }
    }

    void findSenderRoom(Message& msg, ServerData& server)
    {
        std::string     toDisplay;
        size_t          roomIndex = server.m_clientRooms[msg.m_sender];

        toDisplay = server.m_rooms[roomIndex].m_roomName;

        // add username before message
        toDisplay += " | " + server.m_clientNames[msg.m_sender];
        toDisplay += " > " + msg.m_text;
        net::consoleOutput(toDisplay.c_str());

        msg.m_text = toDisplay;
        sendToRoom(msg, server);
    }

    void removeFromRoom(Chatroom& room, const net::Socket& toRemove)
    {
        for (auto iterator = room.m_users.begin();
            iterator != room.m_users.end(); ++iterator)
        {
            if (*iterator == toRemove)
            {
                room.m_users.erase(iterator);
                break;
            }
        }
    }

    void assignRoom(const net::Socket::Handle_t client, ServerData& server)
    {
        if(server.m_rooms.empty())
        {
            server.m_rooms.emplace_back("#general", client);
            server.m_clientRooms[client] = 0;
        }
        if (!server.m_clientRooms.contains(client))
        {
            server.m_clientRooms[client] = 0;
            server.m_rooms[0].m_users.push_back(client);
        }
    }

}
