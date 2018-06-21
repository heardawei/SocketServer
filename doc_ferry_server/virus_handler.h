#pragma once

#include "socket_handler.h"
#include "virus_check.h"


class VirusHandler : public SocketHandler, public vc::VirusCheck
{
public:
	VirusHandler(cp_socket_t sock = INVALID_SOCKET, ss_map_t *p_handlers = nullptr);

	virtual bool readable();
	virtual bool writable();

protected:
	~VirusHandler();
	virtual void handle_expt();
	virtual void handle_read();
	virtual void handle_write();
	virtual void handle_error();
	virtual void handle_close();
};
