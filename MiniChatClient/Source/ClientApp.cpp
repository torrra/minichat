#include "ClientApp.h"

#include <Network/ConsoleOutput.h>
#include <Network/ErrorHandling.h>
#include <Network/Packet.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>

namespace client
{
    ClientApp::~ClientApp(void)
    {
        m_socket.shutdown();
        m_socket.close();
    }

    void ClientApp::welcomeMessage(void)
    {
        net::initConsole();
        SetConsoleTitleA("Mini chat client");
        net::consoleOutput("Enter server IP address: ");
    }

    void ClientApp::update(void)
    {
        //resetConsoleLine();
        ClientEvent clientEvent = waitForMessages();


        switch (clientEvent)
        {
        case IN_ENTER_PRESSED:

            if (!m_connected)
                attemptToConnect();

            else if (m_name.empty())
                registerUsername();

            else
                sendMessage();

            m_input.clear();
            break;

        case SERV_RECEIVE_READY:
            resetConsoleLine();
            net::consoleOutput(receiveMessage().c_str());
            displayUserInput();
            break;

        default: break;

        }

    }


    ClientApp::ClientEvent ClientApp::waitForMessages(void)
    {
        void*           handles[2]{ GetStdHandle(STD_INPUT_HANDLE), m_serverEvent };
        unsigned long   result;
        int             timeout = 100;


        if (m_connected)
            result = WaitForMultipleObjects(2, handles, FALSE, timeout);
        else
            result = WaitForSingleObject(handles[0], timeout);

        switch (result)
        {
        case WAIT_OBJECT_0:
            return processUserInput();

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

        if (m_socket.createClient(m_input, NET_DEFAULT_PORT) == NO_ERROR)
        {
            m_serverEvent = m_socket.createServerEvent();
            m_connected = true;
            net::consoleOutput("Enter username: ");
        }

        else
            net::consoleOutput("Failed to connect. Try again: ");

    }

    void ClientApp::resetConsoleLine(void)
    {
        for (size_t count = 0; count < m_input.size(); ++count)
        {
            net::consoleOutput("\b \b");
        }
    }

    void ClientApp::displayUserInput(void)
    {
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
        else
        {
            m_name = m_input;

            std::string     joinMessage = m_input;

            joinMessage += " has joined.\n";
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
        if (result == 0)
            net::reportWindowsError("ReadConsoleInput", GetLastError());

        if (inputRecord.Event.KeyEvent.bKeyDown)
        {
            char    character = inputRecord.Event.KeyEvent.uChar.AsciiChar;

            switch (character)
            {
            case '\0':
                return IN_REGULAR_KEY_PRESSED;

            case '\r':
                net::consoleOutput("\r\n");
                return IN_ENTER_PRESSED;

            case '\b':
                if (!m_input.empty())
                    m_input.pop_back();

                net::consoleOutput("\b ");
                break;

            default:
                m_input += character;
            }

            net::consoleOutput("%1!c!", character);
            return IN_REGULAR_KEY_PRESSED;
        }

        return NO_EVENT;
    }

    std::string ClientApp::receiveMessage(void)
    {
        char   receivedData[NET_MAX_PACKET_SIZE];

        memset(receivedData, 0, sizeof receivedData);
        m_socket.receive(receivedData, sizeof receivedData);

        size_t      messageLength = net::Packet::findPacketSize(receivedData);

        ResetEvent(m_serverEvent);

        return net::Packet::unpackMessage(receivedData, messageLength);

    }

    void ClientApp::sendMessage(void)
    {
        if (m_input.empty())
            return;

        std::string message = m_name;

        message += " > ";
        message += m_input;
        message += "\r\n\r\n";

        m_socket.send(message.c_str(), static_cast<int>(message.size()));
    }
}