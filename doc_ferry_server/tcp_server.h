#pragma once

#include "base_socket.h"

/*
* instance a server
*
*/
class TCPServerImpl : public BaseSocket
{
public:
	/*
	* p_handlers	:
	* host			: if you do not know how to fill, just nullptr
	*
	* port			: must specific a port to listen
	*/
	TCPServerImpl(const char *host, uint16_t port, ss_map_t *p_handlers);
	virtual void handle_accept();
};

class TCPServer
{
public:
	TCPServer(const char *host, uint16_t port, ss_map_t *p_handlers = nullptr);
	~TCPServer();
protected:
	cp_socket_t p_impl_sock;
	TCPServerImpl *p_impl;
	ss_map_t *p_handlers;
};
