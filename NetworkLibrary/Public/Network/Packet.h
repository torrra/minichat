#pragma once

#include <vector>

#include "Socket.h"

#ifndef NET_MAX_PACKET_SIZE
#define NET_MAX_PACKET_SIZE		1024ull
#endif // !NET_MAX_PACKET_SIZE

namespace net
{
	class Packet
	{
	private:
								Packet(void) = default;
	public:
								Packet(const void* buffer,
									   size_t size, const Socket& sender);

								~Packet(void) = default;

		// Add data at the end of buffer
		bool					append(const void* buffer, size_t size);

		// Accessors

		size_t					getSize(void)							const;
		const char*				getData(void)							const;
		const Socket&			getSender(void)							const;

		// Only use if packet buffer has been zeroed-out before calling receive
		static size_t          findPacketSize(char* packetBuffer);

		// Write packet data into string
		static std::string     unpackMessage(const net::Packet& packet);

		// Write void* and size into string
		static std::string     unpackMessage(const char* packetData, size_t size);


	private:
		char		m_buffer[NET_MAX_PACKET_SIZE];
		size_t		m_size	= 0ull;
		Socket		m_sender;

	};
}