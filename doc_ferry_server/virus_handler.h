#pragma once

#include "socket_handler.h"
#include "interface.h"
#include "HV.h"
#include "chat.h"
#include <queue>
#include <mutex>

class VirusHandler : public SocketHandler, public vc::Chat
{
public:
	VirusHandler(cp_socket_t sock = INVALID_SOCKET, ss_map_t *p_handlers = nullptr);

	virtual bool readable();
	virtual bool writable();

protected:
	
	char *send_buf;
	uint64_t send_capacity;
	
	vc::HV				req;
	std::queue<vc::HV>	resp;
	iface::State		state;

	virtual void request(vc::HV &req);
	virtual void response(vc::HV &req, iface::State state);

	virtual void handle_expt();
	virtual void handle_read();
	virtual void handle_write();
	virtual void handle_error();
	virtual void handle_close();

	~VirusHandler();
};