#include "ClientApp.h"

#include <Network/ConsoleOutput.h>
#include <Network/ErrorHandling.h>
#include <Network/Packet.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>

#ifndef SHUTDOWN_MSG
#define SHUTDOWN_MSG    "jrII0JB0y6"
#endif // !NET_SHUTDOWN_MSG

#ifndef SERVER_ERROR
#define SERVER_ERROR    -1
#endif

namespace client
{
    ClientApp::~ClientApp(void)
    {
    }

    void ClientApp::welcomeMessage(void)
    {
        // Set codepages
        net::initConsole();
        SetConsoleTitleA("Mini chat client");
        net::consoleOutput("Enter server IP address: ");
    }

    void ClientApp::update(bool& running)
    {
        ClientEvent     clientEvent = waitForMessages();

        switch (clientEvent)
        {
        case IN_ENTER_PRESSED:

            // Get server IP
            if (!(m_flags & CONNECTED))
                attemptToConnect();

            // get username if user just connected
            else if (!(m_flags & NAMED))
                registerUsername();

            // send message
            else if (sendMessage() == SERV_SHUTDOWN)
                running = false;

            m_input.clear();
            break;

        // delete user input from console before displaying a
        // new message and re-displaying it
        case SERV_RECEIVE_READY: processIncomingMessage(running); break;

        // Shut down client.
        case IN_ESC_PRESSED:
            net::consoleOutput("Exiting...\n");
            running = false;
            break;

        default: break;

        }

        // terminate connection if we're exiting caller loop
        if (!running && m_socket.isValid())
            m_socket.close();

    }


    ClientApp::ClientEvent ClientApp::waitForMessages(void)
    {
        void*           handles[2]{ GetStdHandle(STD_INPUT_HANDLE), m_serverEvent };
        unsigned long   result;
        int             timeout = static_cast<int>(INFINITE);

        // Wait for server and user input if connected to a valid server
        if (m_flags & CONNECTED)
            result = WaitForMultipleObjects(2, handles, FALSE, timeout);

        // Only poll user input if not
        else
            result = WaitForSingleObject(handles[0], timeout);

        switch (result)
        {
        // new user input
        case WAIT_OBJECT_0:
            return processUserInput();

        // server sent a message
        case WAIT_OBJECT_0 + 1:
            return SERV_RECEIVE_READY;

        case WAIT_FAILED:
            net::reportWindowsError("WaitForMultipleObjects", GetLastError());
            break;

        default:
            break;
        }

        return NO_EVENT;

    }

    void ClientApp::attemptToConnect(void)
    {
        if (m_input.size() == 0)
            return;

        // TODO: remove
        if (m_socket.createClient(m_input, NET_DEFAULT_PORT) == NO_ERROR)
        {
            // Create WSA event for new messages from server
            m_serverEvent = m_socket.createServerEvent();
            m_flags |= CONNECTED;
            net::consoleOutput("Enter username: ");
        }

        else
            net::consoleOutput("Failed to connect. Try again: ");

    }

    void ClientApp::resetConsoleLine(void) const
    {
        // Delete all of user input
        for (size_t count = 0; count < m_input.size(); ++count)
        {
            if (!checkEmptyLine())
                net::consoleOutput("\b \b");
        }
    }

    void ClientApp::displayUserInput(void) const
    {
        // Re-display user input after deletion
        if (m_input.size())
            net::consoleOutput(m_input.c_str());
    }


    void ClientApp::registerUsername(void)
    {
        if (m_input.empty())
        {
            net::consoleOutput("Empty username. Please try again: ");
            return;
        }

        // Tell server new client has successfully joined
        else
        {
            std::string     joinMessage = "/nick ";

            joinMessage += m_input;

            m_flags |= NAMED;
            net::consoleOutput("Welcome, %1\n\n", m_input.c_str());
            m_socket.send(joinMessage.c_str(), static_cast<int>(joinMessage.size()));
        }
    }

    ClientApp::ClientEvent client::ClientApp::processUserInput(void)
    {
        INPUT_RECORD    inputRecord;
        unsigned long   numEvents;
        int             result = ReadConsoleInputA(GetStdHandle(STD_INPUT_HANDLE),
                                                   &inputRecord, 1,
                                                   &numEvents);
        // failed to read from standard input
        if (result == 0)
            net::reportWindowsError("ReadConsoleInput", GetLastError());

        // no relevant user input
        if (!inputRecord.Event.KeyEvent.bKeyDown)
            return NO_EVENT;

            char    character = inputRecord.Event.KeyEvent.uChar.AsciiChar;

        switch (character)
        {
        // no relevant key was pressed
        case '\0': return IN_REGULAR_KEY_PRESSED;
        case '\r':  net::consoleOutput("\r\n"); return IN_ENTER_PRESSED;

        // delete last character from input
        case '\b':

            // Do not delete characters that were entered by user
            if (!m_input.empty())
            {
                m_input.pop_back();

                // Only delete a character if line is not empty
                if (!checkEmptyLine())
                    net::consoleOutput("\b \b");
            }
            return IN_REGULAR_KEY_PRESSED;

        case '\x1B': return IN_ESC_PRESSED;

        default: m_input += character; break;
        }

        net::consoleOutput("%1!c!", character);
        return IN_REGULAR_KEY_PRESSED;
    }

    bool ClientApp::checkEmptyLine(void) const
    {
        CONSOLE_SCREEN_BUFFER_INFO      screenInfo;
        void*                           handle = GetStdHandle(STD_OUTPUT_HANDLE);

        // get window size and cursor positions
        if (!GetConsoleScreenBufferInfo(handle, &screenInfo))
        {
            net::reportWindowsError("GetConsoleScreenBufferInfo", GetLastError());
            return false;
        }


        if (screenInfo.dwCursorPosition.Y < 1)
            return false;

        if (screenInfo.dwCursorPosition.X == 0)
        {
            COORD newPos
            {
                .X = screenInfo.srWindow.Right,
                .Y = screenInfo.dwCursorPosition.Y - 1
            };

            // Move cursor up
            SetConsoleCursorPosition(handle, newPos);
            net::consoleOutput(" ");

            return true;
        }

        return false;
    }

    std::string ClientApp::receiveMessage(void) const
    {
        char   receivedData[NET_MAX_PACKET_SIZE];

        // zero-out buffer before receiving data.
        // needed to get number or non-zero bytes in buffer
        memset(receivedData, 0, sizeof receivedData);
        m_socket.receive(receivedData, sizeof receivedData);

        size_t      messageLength = net::Packet::findPacketSize(receivedData);

        ResetEvent(m_serverEvent);
        return net::Packet::unpackMessage(receivedData, messageLength);

    }

    ClientApp::ClientEvent ClientApp::sendMessage(void) const
    {
        if (m_input.empty())
            return NO_EVENT;

        if (m_input.size() >= NET_MAX_PACKET_SIZE - 4ull)
        {
            net::consoleOutput("\nMessage too long.\n");
            return ClientEvent::NO_EVENT;
        }

        net::Packet     message(m_input.c_str(), m_input.size(), m_socket);

        // send message to server
        message.append("\r\n\r\n", 4ull);

        int     size = static_cast<int>(message.getSize());

        // Disconnect client if send failed
        if (m_socket.send(message.getData(), size) == SERVER_ERROR)
        {
            net::consoleOutput("Lost connection to server. Message not sent.\n");
            return SERV_SHUTDOWN;
        }
        else
            return NO_EVENT;
    }


    ClientApp::ClientEvent ClientApp::checkSpecialMessages(const std::string& message) const
    {
        // look for special shutdown message
        if (message.compare(SHUTDOWN_MSG) == 0)
            return SERV_SHUTDOWN;

        else
            return NO_EVENT;
    }

    void ClientApp::processIncomingMessage(bool& running) const
    {
        std::string     message = receiveMessage();

        switch (checkSpecialMessages(message))
        {
        // shutdown message received
        case SERV_SHUTDOWN:
            running = false;
            net::consoleOutput("Server disconnected. Shutting down client.\n");
            break;

        // normal message received
        default:

            // do not display message if user has
            // not entered a username yet
            if (!(m_flags & NAMED))
                return;

            resetConsoleLine();
            net::consoleOutput(message.c_str());
            displayUserInput();
            break;
        }
    }
}