#pragma once

#include "socket_handler.h"
#include "virus_check.h"


class VirusHandler : public SocketHandler, public vc::VirusCheck
{
public:
	VirusHandler(cp_socket_t sock = INVALID_SOCKET, ss_map_t *p_handlers = nullptr);

	virtual bool writable();
	virtual bool readable();

protected:
	~VirusHandler();
	virtual void handle_read();
};
