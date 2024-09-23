#include "Packet.h"
#include "ErrorHandling.h"
#include "ConsoleOutput.h"
#include "Socket.h"

namespace net
{
	Packet::Packet(const void* buffer, size_t size, const Socket& sender)
		: m_size(size), m_sender(sender)
	{
		// Copy buffer into packet buffer
		if (size <= NET_MAX_PACKET_SIZE)
			memcpy(m_buffer, buffer, size);
		else
		{
			// Zero-out everything on failure
			consoleOutput("Packet error: not enough buffer space\n");
			memset(m_buffer, 0, sizeof m_buffer);
			m_size = 0;
		}
	}

	bool Packet::append(const void* buffer, size_t size)
	{
		// Add data if ther is enough space
		if (m_size + size <= NET_MAX_PACKET_SIZE)
		{
			memcpy(m_buffer + m_size, buffer, size);
			m_size += size;
			return true;
		}
		else
		{
			// Print error message if incoming buffer is too long
			consoleOutput("Packet error: not enough buffer space\n");
			return false;
		}
	}

	size_t Packet::getSize(void) const
	{
		return m_size;
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
		const char*		rawData = packet.getData();

		// Make room for packet data
		message.resize(packet.getSize());

		// Copy packet data into string
		for (size_t index = 0; index < packet.getSize(); ++index)
		{
			message[index] = rawData[index];
		}

		return message;
	}

	std::string Packet::unpackMessage(const char* packetData, size_t size)
	{
		std::string     message;

		// Make room for packet data
		message.resize(size);

		// Copy packet data into string
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

		// Count characters until 0 or max buffer size
		while (data && count < NET_MAX_PACKET_SIZE - 1ull)
		{
			++count;
			data = packetBuffer[count];
		}

		return count;
	}
}
