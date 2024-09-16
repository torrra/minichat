#pragma once

namespace net
{
	// Size = 16B, padding = 0B

	struct Packet
	{
		Packet(void) = default;
		Packet(size_t size, void* data);

		size_t	m_bufferSize	= 0ull;
		void*	m_buffer		= nullptr;
	};
}