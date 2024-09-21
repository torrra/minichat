#pragma once

#include <vector>

#include "Socket.h"

#ifndef NET_MAX_PACKET_SIZE
#define NET_MAX_PACKET_SIZE		512ull
#endif // !NET_MAX_PACKET_SIZE

namespace net
{
	// Size = 16B, padding = 0B

	class Packet
	{
	private:
						Packet(void) = default;
	public:
						Packet(void* buffer, size_t size, const Socket& sender);
						~Packet(void) = default;
		bool			append(void* buffer, size_t size);
		size_t			getSize(void)							const;

		const char*		getData(void)							const;
		const Socket&	getSender(void)							const;

		// Only use if packet buffer has been zeroed-out before calling receive
		static size_t          findPacketSize(char* packetBuffer);
		static std::string     unpackMessage(const net::Packet& packet);
		static std::string     unpackMessage(const char* packetData, size_t size);


	private:
		char		m_buffer[NET_MAX_PACKET_SIZE];
		size_t		m_bufferSize	= 0ull;
		Socket		m_sender;

	};
}