#include "debug_print.h"
#include "tcp_server.h"
#include "virus_handler.h"

TCPServerImpl::TCPServerImpl(const char *host, uint16_t port, ss_map_t *p_handlers) :
	BaseSocket(INVALID_SOCKET, p_handlers)
{
	this->create_socket(AF_INET, SOCK_STREAM);
	this->set_reuse_addr();
	this->bind(ss_addr_t(std::string(host), port));
	this->listen(5);
}

void TCPServerImpl::handle_accept()
{
	ss_addr_t addr;
	cp_socket_t sock = this->accept(&addr);
	if (cp_socket_valid(sock))
	{
		// TODO, support multi-thread/multi-process

		// this pointer(SocketHandler *) will insert self into this->p_handlers;
		new VirusHandler (sock, this->p_handlers);
		debug_print("accept a new client, sock: %d\n", sock);
	}
	else
	{
#ifdef CP_WINDOWS
		debug_print("accept new client failed, errcode: %d\n", WSAGetLastError());
#else CP_LINUX
		debug_print("accept new client failed, errcode: %d\n", errno);
#endif /* CP_WINDOWS/CP_LINUX */
	}
}

TCPServer::TCPServer(const char *host, uint16_t port, ss_map_t *p_handlers) :
	p_handlers(p_handlers)
{
	if (this->p_handlers == nullptr)
	{
		this->p_handlers = &g_handlers;
	}
	if (host == nullptr)
	{
		host = "";
	}
	this->p_impl = new TCPServerImpl(host, port, this->p_handlers);
	if (this->p_impl)
	{
		p_impl_sock = this->p_impl->socket();
	}
}

TCPServer::~TCPServer()
{
	p_handlers->erase(p_impl_sock);
}