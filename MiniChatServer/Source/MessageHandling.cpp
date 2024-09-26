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

            // send message to other clients if it is not a command
            if(!checkCommands(message, data, running))
            {
                net::Socket::Handle_t   sender = received.getSender().getHandle();
                std::string             toDisplay = data.m_clientNames[sender];

                // ass username before message
                toDisplay += " > ";
                toDisplay += message.m_text;

                net::consoleOutput(toDisplay.c_str());

                // send message to all clients except sender
                data.m_server.createPacket(toDisplay.c_str(),
                                           toDisplay.size(),
                                           message.m_sender);


            }
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

    void removeNewlines(std::string& username)
    {
        for (size_t index = username.size() - 1ull; index > 0ull; --index)
        {
            // delete newline characters and leave at least
            // 1 character in string
            switch (username[index])
            {
            case '\n':
                username.pop_back();
                continue;

            case '\r':
                username.pop_back();
                continue;

            default: return;
            }
        }
    }

}
