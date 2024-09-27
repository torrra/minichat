#pragma once

#include <Network/Socket.h>

#include <string>

namespace client
{
    class ClientApp
    {
    private:

        // client event value obtained from event polling
        // IN_ prefix: input event
        // SERV_ prefix: server event
        enum ClientEvent
        {
            NO_EVENT,
            IN_REGULAR_KEY_PRESSED,
            IN_ENTER_PRESSED,
            SERV_RECEIVE_READY,
            SERV_SHUTDOWN
        };

        enum ClientFlag : unsigned char
        {
            CONNECTED   = 1,
            NAMED       = 1 << 1
        };

    public:
                ClientApp(void) = default;
                ~ClientApp(void);

        // Ask user for server IP address
        void     welcomeMessage(void);

        // Poll events and handle messages
        void     update(bool& running);

    private:

        // Connect to server

        void            attemptToConnect(void);
        void            registerUsername(void);

        // Move user input when receiving a message

        void            resetConsoleLine(void)                              const;
        void            displayUserInput(void)                              const;


        // Wait for input or server event
        ClientEvent     waitForMessages(void);

        // Read user input
        ClientEvent     processUserInput(void);
        bool            checkEmptyLine(void)        const;

        // Message handling

        std::string     receiveMessage(void)                                const;
        ClientEvent     sendMessage(void)                                   const;

        ClientEvent     checkSpecialMessages(const std::string& message)    const;
        void            processIncomingMessage(bool& running)               const;

        // Client socket handle
        net::Socket     m_socket;

        // Current user input
        std::string     m_input;

        // Server event for polling
        void*           m_serverEvent = nullptr;

        // Is this client connected to a server with a username ?
        unsigned char   m_flags = 0;
    };
}