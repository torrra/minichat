#pragma once

#include <Network/Server.h>

namespace server
{
	void processMessages(net::Server& server, bool& running);
}