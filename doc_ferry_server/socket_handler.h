#pragma once
#include "base_socket.h"

/*
 * instance a handler for a cp_socket_t
 * this instance will insert self into p_handlers automically
 */
class SocketHandler : public BaseSocket
{
public:
	SocketHandler(cp_socket_t sock = INVALID_SOCKET, ss_map_t *p_handlers = nullptr);
	~SocketHandler();
	virtual void handle_expt();
	virtual void handle_read();
	virtual void handle_write();
	virtual void handle_close();

	virtual bool readable();
	virtual bool writable();
};