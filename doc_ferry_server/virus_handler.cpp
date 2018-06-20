#include "virus_handler.h"
#include "debug_print.h"



VirusHandler::VirusHandler(cp_socket_t sock, ss_map_t *p_handlers) :
	SocketHandler(sock, p_handlers)
{
	debug_print("new client: %s:%d\n", this->addr.first.data(), this->addr.second);
}


VirusHandler::~VirusHandler()
{
	debug_print("del client: %s:%d\n", this->addr.first.data(), this->addr.second);
}


bool VirusHandler::readable()
{
	return cp_socket_valid(this->socket());
}


bool VirusHandler::writable()
{
	return false;
}


void VirusHandler::handle_read()
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

	debug_print("read from [%s:%d] [%dB]\n", this->addr.first.data(), this->addr.second, retv);

	this->on_packet(buf, retv);

	if (this->resps.size())
	{
		vc::Response rsp = this->resps.front();
		this->resps.pop();

		int n_send = cp_send(this->sock, (char *)&rsp.header, (int)sizeof(rsp.header), 0);
		debug_print("send to   [%s:%d] [%dB]\n", this->addr.first.data(), this->addr.second, n_send);
		cp_send(this->sock, (char *)&rsp.errcode, (int)sizeof(rsp.errcode), 0);
		debug_print("send to   [%s:%d] [%dB]\n", this->addr.first.data(), this->addr.second, n_send);
	}

	return;
}
