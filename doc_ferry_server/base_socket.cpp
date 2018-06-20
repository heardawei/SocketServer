#include <vector>
#include <memory>

#include "debug_print.h"
#include "cp_socket.h"
#include "base_socket.h"

/* ensure all things for start up done before main invoked */
class Startup
{
public:
	Startup()
	{
		cp_socket_init();
	}
	~Startup()
	{
		cp_socket_uninit();
	}
};

static Startup start_up;
ss_map_t g_handlers;

void BaseSocket::add_channel(ss_map_t *p_handlers)
{
	p_handlers->erase(this->sock);
	auto_BaseSocket_t ptr(this);
	std::pair<cp_socket_t, auto_BaseSocket_t> pair(this->sock, ptr);
	p_handlers->insert(pair);
//	(*p_handlers)[this->sock] = std::shared_ptr<BaseSocket> (this);
	debug_print("add_channel socket:%d\n", (int)this->sock);
}

void BaseSocket::del_channel(ss_map_t *p_handlers)
{
	debug_print("del_channel socket:%d\n", (int)this->sock);
	p_handlers->erase(this->sock);
}

int BaseSocket::create_socket(int af, int type)
{
	this->family_type = std::pair<int, int>(af, type);
	if (type == SOCK_STREAM)
	{
		this->sock = cp_socket_tcp();
		debug_print("%s creating tcp socket\n", __func__);
	}
	else if (type == SOCK_DGRAM)
	{
		this->sock = cp_socket_udp();
		debug_print("%s creating udp socket\n", __func__);
	}
	if (cp_socket_valid(this->sock))
	{
		cp_setblocking(this->sock, false);
		this->set_socket(this->sock, this->p_handlers);
		debug_print("%s success, create socket:%d\n", __func__, (int)this->sock);
		return 0;
	}
#ifdef CP_WINDOWS
	int errcode = WSAGetLastError();
	debug_print("%s failed, create a invalid socket:%d, errno:%d\n", __func__, (int)this->sock, errcode);
#elif CP_LINUX
	debug_print("%s failed, create a invalid socket:%d\n", __func__, (int)this->sock);
#endif /* CP_WINDOWS/CP_LINUX */
	return -1;
}

void BaseSocket::set_socket(cp_socket_t sock, ss_map_t *p_handlers)
{
	this->sock = sock;
	this->add_channel(p_handlers);
}

void BaseSocket::set_reuse_addr()
{
	// TODO
}

BaseSocket::BaseSocket(cp_socket_t sock, ss_map_t *p_handlers) :
	sock(sock), p_handlers(p_handlers), connected(false), connecting(false), accepting(false), closing(false), debug(true)
{
	if (this->p_handlers == nullptr)
	{
		this->p_handlers = &g_handlers;
	}
	if (cp_socket_valid(sock))
	{
		cp_setblocking(sock, false);
		this->set_socket(sock, p_handlers);
		this->connected = true;
		struct sockaddr_in sin;
		if (cp_getpeername(sock, &sin) == 0)
		{
			char ip[16] = { 0 };
			std::string peer_host = cp_inet_ntop(&sin, ip, sizeof(ip));
			uint16_t peer_port = ntohs(sin.sin_port);
			this->addr = ss_addr_t(peer_host, peer_port);
			debug_print("sock:%d getpeername -> %s:%d\n", sock, peer_host.data(), peer_port);
		}
		else
		{
#ifdef CP_WINDOWS
			int errcode = WSAGetLastError();
			debug_print("sock:%d getpeername failed, errcode:%d\n", sock, errcode);
			if (errcode == WSAENOTCONN)
			{
				this->connected = false;
			}
#elif CP_LINUX
			debug_print("sock:%d getpeername failed, errcode:%d\n", sock, errno);
			if (errno = EINVAL || ENOTCONN)
			{
				this->connected = false;
			}
#endif /* CP_WINDOWS/CP_LINUX */
			else
			{
				// TODO: raise
				this->del_channel(this->p_handlers);
				// TODO: from now, <this> pointer is invalid
				return;
			}
		}
	}
}

BaseSocket::~BaseSocket()
{
}

cp_socket_t BaseSocket::socket()
{
	return this->sock;
}

ss_addr_t BaseSocket::address()
{
	return this->addr;
}

bool BaseSocket::is_accepting()
{
	return accepting;
}

bool BaseSocket::readable()
{
	return cp_socket_valid(this->socket());
}

bool BaseSocket::writable()
{
	return cp_socket_valid(this->socket());
}

int BaseSocket::listen(int maxn)
{
	this->accepting = true;
	// TODO, on windows NT, maxn is less than 5
	return cp_listen(this->sock, maxn);
}

int BaseSocket::bind(ss_addr_t addr, int af)
{
	this->addr = addr;
	return cp_bind(this->sock, af, addr.first.size() ? addr.first.data() : nullptr, addr.second);
}

int BaseSocket::connect(ss_addr_t addr, int af)
{
	this->connected = false;
	this->connecting = true;
	int ret = cp_connect(this->sock, af, addr.first.size() ? addr.first.data() : nullptr, addr.second);
	if (ret == 0/* || err_is_conn */)
	{
		this->addr = addr;
		this->handle_connect_event();
		return 0;
	}
	/* check errno base OS */
#ifdef CP_WINDOWS
	int errcode = WSAGetLastError();
	if (errcode == WSAEISCONN)
	{
		this->addr = addr;
		this->handle_connect_event();
		return 0;
	}
	else if (errcode == WSAEINPROGRESS || errcode == WSAEALREADY || errcode == WSAEWOULDBLOCK)
	{
		this->addr = addr;
	}
	else
	{
		// TODO raise
	}
#elif CP_LINUX
	if (errcode == EISCONN)
	{
		this->addr = addr;
		this->handle_connect_event();
		return 0;
	}
	else if (errno == EALREADY || errno == EINPROGRESS)
	{
		this->addr = addr;
		return -1;
	}
	else
	{
		// TODO raise
	}
#endif /* CP_WINDOWS/CP_LINUX */

	return -1;
}

cp_socket_t BaseSocket::accept(ss_addr_t *addr)
{
	struct sockaddr_in sin_addr;
	cp_socket_t sock = cp_accept(this->sock, &sin_addr);
	if (cp_socket_valid(sock))
	{
		char ip[16] = { 0 };
		cp_inet_ntop(&sin_addr, ip, sizeof(ip));
		uint16_t port = ntohs(sin_addr.sin_port);
		if (addr)
		{
			*addr = ss_addr_t(ip, port);
		}
	}
	else
	{
#ifdef CP_WINDOWS
		int errcode = WSAGetLastError();
		if (errcode == WSAEWOULDBLOCK)
		{
			return INVALID_SOCKET;
		}
		else
		{
			// TODO raise
		}
#elif CP_LINUX
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == ECONNABORTED)
		{
			return INVALID_SOCKET;
		}
		else
		{
			// TODO raise
		}
#endif /* CP_WINDOWS/CP_LINUX */
	}
	return sock;
}

int BaseSocket::send(const char *buf, size_t size)
{
	int ret = cp_send(this->sock, buf, (int)size);
	// TODO check errno
	if (ret == SOCKET_ERROR)
	{
#ifdef CP_WINDOWS
		int errcode = WSAGetLastError();
		if (errcode == WSAEWOULDBLOCK)
		{
			ret = 0;
		}
		else
		{
			this->handle_close();
		}
#elif CP_LINUX
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			ret = 0;
		}
		else
		{
			this->handle_close();
		}
#endif /* CP_WINDOWS/CP_LINUX */
	}
	return ret;
}

int BaseSocket::recv(char *buf, size_t size)
{
	int ret = cp_recv(this->sock, buf, (int)size);
	// TODO check errno
	if (ret == 0)
	{
		this->handle_close();
	}
	else if (ret == SOCKET_ERROR)
	{
		this->handle_close();
	}
	return ret;
}

void BaseSocket::close()
{
	this->connected = false;
	this->connecting = false;
	this->accepting = false;
	this->del_channel(this->p_handlers);
	cp_close_socket(this->sock);
}

void BaseSocket::log(std::string messg)
{
	fprintf(stderr, "%s", messg.data());
}

void BaseSocket::log_warning(std::string msg)
{
	msg = std::string("WARNING: ") + msg;
	this->log(msg);
}

void BaseSocket::log_info(std::string msg)
{
	msg = std::string("INFO: ") + msg;
	this->log(msg);
}

void BaseSocket::handle_read_event()
{
	if (this->accepting)
	{
		this->handle_accept();
	}
	else if (!this->connected)
	{
		if (this->connecting)
		{
			this->handle_connect_event();
		}
		this->handle_read();
	}
	else
	{
		this->handle_read();
	}
}

void BaseSocket::handle_write_event()
{
	if (this->accepting)
	{
		return;
	}
	if (!this->connected)
	{
		if (this->connecting)
		{
			this->handle_connect_event();
		}
	}
	this->handle_write();
}

void BaseSocket::handle_connect_event()
{
	int err = getsockopt(this->sock, SOL_SOCKET, SO_ERROR, nullptr, nullptr);
	if (err)
	{
		this->handle_error();
		return;
	}
	this->handle_connect();
	this->connected = true;
	this->connecting = false;
}

void BaseSocket::handle_expt_event()
{
	int err = getsockopt(this->sock, SOL_SOCKET, SO_ERROR, nullptr, nullptr);
	if (err == 0)
	{
		this->handle_expt();
	}
	else
	{
		this->handle_close();
	}
}

void BaseSocket::handle_error()
{
	// TODO, print err msg
	this->handle_close();
}

void BaseSocket::handle_expt()
{
	this->log_warning("unhandled incoming priority event\n");
	this->handle_close();
}

void BaseSocket::handle_read()
{
	this->log_warning("unhandled read event\n");
}

void BaseSocket::handle_write()
{
	this->log_warning("unhandled write event\n");
}

void BaseSocket::handle_connect()
{
	this->log_warning("unhandled connect event\n");
}

void BaseSocket::handle_accept()
{
	this->log_warning("unhandled accept event\n");
}

void BaseSocket::handle_close()
{
	this->log_warning("unhandled close event\n");
	this->close();
}


/* ------------------------------------ MainProcess ------------------------------------ */

static void do_select(int timeout_ms, ss_map_t *p_handlers)
{
	fd_set r, w, e;
	std::vector<cp_socket_t> select_socks;

	cp_socket_t max_fd = 0;

	struct timeval tv;

	debug_print("total handlers:%d\n", p_handlers->size());

	if (p_handlers->size() == 0)
	{
		return;
	}

	/* init select arguments */

	FD_ZERO(&r);
	FD_ZERO(&w);
	FD_ZERO(&e);

	for (auto &s : *p_handlers)
	{
		const cp_socket_t &sock = s.first;
		BaseSocket *p_handler = s.second.get();

		bool isr = p_handler->readable();
		bool isw = p_handler->writable();

		if (isr)
		{
			FD_SET(sock, &r);
			if (max_fd < sock)
			{
				max_fd = sock;
			}
		}
		/* accepting socket should not be writable */
		if (isw && !p_handler->is_accepting())
		{
			FD_SET(sock, &w);
			if (max_fd < sock)
			{
				max_fd = sock;
			}
		}
		if (isr || isw)
		{
			FD_SET(sock, &e);
			select_socks.push_back(sock);
			if (max_fd < sock)
			{
				max_fd = sock;
			}
		}
	}

	debug_print("total select_socks:%d\n", (int)select_socks.size());

	if (max_fd == 0)
	{
#ifdef CP_WINDOWS
		Sleep(timeout_ms);
#elif CP_LINUX
		usleep(timeout_ms * 1000);
#endif /* CP_WINDOWS/CP_LINUX */
		return;
	}

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = timeout_ms % 1000 * 1000;
#ifdef CP_WINDOWS
	int retv = select(FD_SETSIZE, &r, &w, &e, &tv);
#elif CP_LINUX
	int retv = select(max_fd + 1, &r, &w, &e, &tv);
#endif /* CP_WINDOWS/CP_LINUX */
	if (retv == 0)
	{
		debug_print("select timeout\n");
		return;
	}
	else if (retv == SOCKET_ERROR)
	{
#ifdef CP_WINDOWS
		int errcode = WSAGetLastError();
		debug_print("select a error: %d\n", errcode);
		if (errcode == WSAEINTR)
		{
			return;
		}
#elif CP_LINUX
		if (errno == EINTR)
		{
			return;
		}
#endif /* CP_WINDOWS/CP_LINUX */
		return;
	}

	/* after select */
	debug_print("total active_socks:%d\n", retv);
	for (cp_socket_t &sock : select_socks)
	{
		if (retv > 0)
		{
			auto itr = p_handlers->find(sock);
			if (itr == p_handlers->end())
			{
				continue;
			}

			if (retv > 0 && FD_ISSET(sock, &r) && itr->second->readable())
			{
				itr->second->handle_read_event();
				retv--;
			}
		}
		if (retv > 0)
		{
			auto itr = p_handlers->find(sock);
			if (itr == p_handlers->end())
			{
				continue;
			}
			if (retv > 0 && FD_ISSET(sock, &w) && itr->second->writable())
			{
				itr->second->handle_write_event();
				retv--;
			}
		}
		if (retv > 0)
		{
			auto itr = p_handlers->find(sock);
			if (itr == p_handlers->end())
			{
				continue;
			}
			if (retv > 0 && FD_ISSET(sock, &e) && (itr->second->readable() || itr->second->writable()))
			{
				itr->second->handle_expt_event();
				retv--;
			}
		}
	}
	select_socks.clear();
}


void loop(int timeout_ms, size_t count, ss_map_t *p_handlers)
{
	bool no_stop = false;
	if (count == 0)
	{
		no_stop = true;
	}

	if (p_handlers == nullptr)
	{
		debug_print("use global default map\n");
		p_handlers = &g_handlers;
	}
	else
	{
		debug_print("use specific map\n");
	}

	if (p_handlers->size() == 0)
	{
		debug_print("fatal error, specific map is empty, instance a server at first!");
		return;
	}

	if (count == 0)
	{
		while (p_handlers->size())
		{
			do_select(timeout_ms, p_handlers);
		}
	}
	else
	{
		for (size_t i = 0; i < count && p_handlers->size(); i++)
		{
			debug_print("loop %d/%d times\n", i, count);
			do_select(timeout_ms, p_handlers);
		}
		debug_print("after loop %d times, stop loop\n", (int)count);
	}

	return;
}