#include "socket_handler.h"
#include "debug_print.h"
#include "interface.h"

SocketHandler::SocketHandler(cp_socket_t sock, ss_map_t *p_handlers) :
	BaseSocket(sock, p_handlers)
{
	cp_setblocking(sock, true);
}

SocketHandler::~SocketHandler()
{
}

bool SocketHandler::readable()
{
	return cp_socket_valid(this->socket());
}

bool SocketHandler::writable()
{
	return false;
}

void SocketHandler::handle_read()
{
	char buf[4096];

	int retv = cp_recv(this->sock, buf, sizeof(buf), 0);
	if (retv == 0)
	{
		this->handle_close();
		return;
	}
	else if (retv == -1)
	{
		this->handle_expt();
		return;
	}

//	printf("read from [%s:%d] [%dB] [%s]\n", this->addr.first.data(), this->addr.second, retv, buf);
	
//	iface::Response resp;
//	resp.header.id = htonl(1);
//	resp.header.length = htonl(sizeof(int));
//	resp.errcode = htonl(0);
//	cp_send(this->sock, (char *)&resp, sizeof(resp), 0);
//	printf("send to   [%s:%d] [%dB] [%s]\n", this->addr.first.data(), this->addr.second, retv, buf);

	return;
}

void SocketHandler::handle_write()
{
}

void SocketHandler::handle_expt()
{
	this->handle_close();
}

void SocketHandler::handle_error()
{
	this->handle_close();
}

void SocketHandler::handle_close()
{
	this->close();
}