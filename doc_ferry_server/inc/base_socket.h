#pragma once

#include <map>
#include <string>
#include <inttypes.h>
#include <memory>

#include "cp_socket.h"

#define SOCK_STATE_ESTABLISH		(1 << 1)
#define SOCK_STATE_ACCEPTING		(1 << 2)
#define SOCK_STATE_DISCONNECT		(1 << 3)
#define SOCK_STATE_EXCEPTION		(1 << 4)

class BaseSocket;
using ss_addr_t = std::pair<std::string, uint16_t>;
using auto_BaseSocket_t = std::shared_ptr<BaseSocket>;
using ss_map_t = std::map<cp_socket_t, auto_BaseSocket_t>;

/*
* Basic Socket Handler
* implement any handle_xxx what you need by inherit this class
*/
class BaseSocket
{
public:
	BaseSocket(cp_socket_t sock = INVALID_SOCKET, ss_map_t *p_handlers = nullptr);

	cp_socket_t socket();
	ss_addr_t address();
	bool is_accepting();

	void handle_read_event();
	void handle_write_event();
	void handle_connect_event();
	void handle_expt_event();

	virtual bool readable();
	virtual bool writable();

	void add_channel(ss_map_t *p_handlers = nullptr);
	void del_channel(ss_map_t *p_handlers = nullptr);
	void set_socket(cp_socket_t sock, ss_map_t *p_handlers = nullptr);
	void set_reuse_addr();
	int create_socket(int af = AF_INET, int type = 0);
	int listen(int maxn = 64);
	int bind(ss_addr_t addr, int af = AF_INET);
	int connect(ss_addr_t peer_addr, int af = AF_INET);
	cp_socket_t accept(ss_addr_t *addr = nullptr);
	int send(const char *buf, size_t size);
	int recv(char *buf, size_t size);
	void close();
	void log(std::string msg);
	void log_warning(std::string msg);
	void log_info(std::string msg);

	virtual void handle_error();
	virtual void handle_expt();
	virtual void handle_read();
	virtual void handle_write();
	virtual void handle_connect();
	virtual void handle_accept();
	virtual void handle_close();

	virtual ~BaseSocket();

protected:
	cp_socket_t			sock;
	int					state;

	std::pair<int, int> family_type;
	ss_addr_t			addr;
	ss_map_t			*p_handlers;
	bool				connected;
	bool				connecting;
	bool				accepting;
	bool				closing;
	bool				debug;
};

extern ss_map_t g_handlers;

void loop(int timeout_ms = 1000, size_t count = 0, ss_map_t *p_handlers = nullptr);