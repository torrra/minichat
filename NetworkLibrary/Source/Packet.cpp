#include "Packet.h"
#include "ErrorHandling.h"
#include "ConsoleOutput.h"
#include "Socket.h"

namespace net
{
	Packet::Packet(void* buffer, size_t size, const Socket& sender)
		: m_bufferSize(size), m_sender(sender)
	{
		if (size <= NET_MAX_PACKET_SIZE)
			memcpy(m_buffer, buffer, size);
		else
		{
			consoleOutput("Packet error: not enough buffer space\n");
			memset(m_buffer, 0, sizeof m_buffer);
		}
	}

	bool Packet::append(void* buffer, size_t size)
	{
		if (size <= NET_MAX_PACKET_SIZE)
		{
			memcpy(m_buffer, buffer, size);
			return true;
		}
		else
		{
			consoleOutput("Packet error: not enough buffer space\n");
			return false;
		}
	}

	size_t Packet::getSize(void) const
	{
		return m_bufferSize;
	}

	const char* Packet::getData(void) const
	{
		return m_buffer;
	}

	const Socket& Packet::getSender(void) const
	{
		return m_sender;
	}

	std::string Packet::unpackMessage(const net::Packet& packet)
	{
		std::string     message;
		const char* rawData = packet.getData();

		message.resize(packet.getSize());

		for (size_t index = 0; index < packet.getSize(); ++index)
		{
			message[index] = rawData[index];
		}

		return message;
	}

	std::string Packet::unpackMessage(const char* packetData, size_t size)
	{
		std::string     message;

		message.resize(size);

		for (size_t index = 0; index < size; ++index)
		{
			message[index] = packetData[index];
		}

		return message;
	}

	size_t Packet::findPacketSize(char* packetBuffer)
	{
		size_t  count = 0ull;
		char    data = packetBuffer[0];

		while (data)
		{
			++count;
			data = packetBuffer[count];
		}

		return count;
	}
}
