#pragma once

#include <Network/Socket.h>

#include <string>

namespace client
{
    class ClientApp
    {
    private:
        enum ClientEvent
        {
            NO_EVENT,
            IN_REGULAR_KEY_PRESSED,
            IN_ENTER_PRESSED,
            SERV_RECEIVE_READY
        };

    public:
                ClientApp(void) = default;
                ~ClientApp(void);

        void     welcomeMessage(void);
        void     update(bool& running);

    private:

        void            attemptToConnect(void);
        void            resetConsoleLine(void);
        ClientEvent     waitForMessages(void);
        void            displayUserInput(void);
        void            registerUsername(void);
        ClientEvent     processUserInput(void);
        std::string     receiveMessage(void);

        void     sendMessage(void);

        net::Socket     m_socket;
        std::string     m_name;
        std::string     m_input;
        void*           m_serverEvent = nullptr;
        bool            m_connected = false;
    };
}