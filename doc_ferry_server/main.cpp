#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include <string>

#include "debug_print.h"
#include "tcp_server.h"
#include "interface.h"


int main(void)
{
	debug_print("interface header size:%u\n", (unsigned)sizeof(iface::Header));
	std::map<cp_socket_t, std::shared_ptr<BaseSocket>> sock_handlers;
	TCPServer srv(NULL, 7890, &sock_handlers);
	loop(60000, 0, &sock_handlers);

	getchar();
	return 0;
}



#if 0
static cp_socket_t create_server(const char *local_host, uint16_t local_port)
{
	cp_socket_t sock = cp_socket_tcp();
	if (sock == INVALID_SOCKET)
	{
		printf("socket error !");
		return -1;
	}

	if (cp_bind(sock, AF_INET, local_host, local_port) == SOCKET_ERROR)
	{
		printf("bind %s:%d error\n", local_host ? local_host : "NULL", local_port);
		cp_close_socket(sock);
		return -1;
	}

	if (cp_listen(sock, 64) == SOCKET_ERROR)
	{
		printf("listen error\n");
		cp_close_socket(sock);
		return -1;
	}

	debug_print("server on %s:%d is ready\n", local_host ? local_host : "NULL", local_port);
	return sock;
}

static std::map<cp_socket_t, SocketHandler> handlers;

int main(int argc, const char *argv[])
{
	const char *host = NULL;
	uint16_t port = 8898;
	int ret = 0;

	fd_set fs_read;
	fd_set fs_write;


	cp_socket_init();

	if (argc == 2)
	{
		port = atoi(argv[1]);
	}

	cp_socket_t sock = create_server(host, port);
	if (sock == -1)
	{
		fprintf(stderr, "create_server at (%s:%d) failed\n", host ? host : "NULL", port);
		ret = -1;
		goto main_end;
	}

#ifdef CP_LINUX
	int max_fd;
#endif /* CP_LINUX */

	while (true)
	{
		FD_ZERO(&fs_read);
		FD_ZERO(&fs_write);

		FD_SET(sock, &fs_read);
#ifdef CP_LINUX
		max_fd = sock;
#endif /* CP_LINUX */

		for (auto &s : handlers)
		{
			if (s.second.readable())
			{
				FD_SET(s.first, &fs_read);
#ifdef CP_LINUX
				if (max_fd < s.sock)
				{
					max_fd = s.sock;
				}
#endif /* CP_LINUX */
			}
			if (s.second.writable())
			{
				FD_SET(s.first, &fs_write);
#ifdef CP_LINUX
				if (max_fd < s.sock)
				{
					max_fd = s.sock;
				}
#endif /* CP_LINUX */
			}
		}
#ifdef CP_WINDOWS
		int retv = select(FD_SETSIZE, &fs_read, &fs_write, NULL, NULL);
		if (retv == SOCKET_ERROR)
		{
			fprintf(stderr, "select return error: %d\n", retv);
			break;
		}
#elif CP_LINUX
		int retv = select(max_fd + 1, &fs_read, &fs_write, NULL, NULL);
		if (retv == -1)
		{
			fprintf(stderr, "select return error: %d\n", retv);
			break;
		}
#endif /* CP_WINDOWS/CP_LINUX */
		if (FD_ISSET(sock, &fs_read))
		{
			struct sockaddr_in cli_addr;
			cp_socket_t s = cp_accept(sock, &cli_addr);
			if (cp_socket_valid(s))
			{
				SocketHandler cli(s, &cli_addr);
				handlers[s] = cli;
				debug_print("connected    peer [%15s:%5d] fd:[%d]\n",
					cli.address().first.data(), cli.address().second, cli.socket());
			}
			else
			{
				fprintf(stderr, "%s line:%d accept error\n", __func__, __LINE__);
			}
			if (--retv == 0)
			{
				continue;
			}
		}
		// for (std::vector<SocketHandler>::iterator itr = handlers.begin(); itr != handlers.end(); )
		for (auto itr = handlers.begin(); itr != handlers.end(); )
		{
			if (FD_ISSET(itr->first, &fs_read) && itr->second.readable())
			{
				itr->second.handle_read();
				if (itr->second.is_close())
				{
					debug_print("disconnected peer [%15s:%5d] fd:[%d]\n", 
						itr->second.address().first.data(), itr->second.address().second, itr->second.socket());
					cp_close_socket(itr->second.socket());
					itr = handlers.erase(itr);
					continue;
				}
				if (--retv == 0)
				{
					break;
				}
			}
			if (FD_ISSET(itr->first, &fs_write) && itr->second.writable())
			{
				itr->second.handle_write();
				if (itr->second.is_close())
				{
					debug_print("disconnected peer [%15s:%5d] fd:[%d]\n",
						itr->second.address().first.data(), itr->second.address().second, itr->second.socket());
					cp_close_socket(itr->second.socket());
					itr = handlers.erase(itr);
					continue;
				}
				if (--retv == 0)
				{
					break;
				}
			}
			++itr;
		}
	}

main_end:

	cp_socket_uninit();

	printf("input a enter to quit..");
	getchar();
	return ret;
}
#endif


