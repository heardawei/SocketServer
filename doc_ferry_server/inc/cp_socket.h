#pragma once

#ifdef _WIN32
#define CP_WINDOWS		1
#define CP_WINDOWS_32	1
#elif _WIN64
#define CP_WINDOWS		1
#define CP_WINDOWS_64	1
#elif __linux__
#define CP_LINUX		1
#endif


#ifdef CP_WINDOWS
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
typedef SOCKET			cp_socket_t;
typedef int				socklen_t;
#elif CP_LINUX
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
typedef int				cp_socket_t;
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#else /* CP_WINDOWS/CP_LINUX */
#error "Unimplement Socket API On This OS"
#endif /* CP_WINDOWS/CP_LINUX */

#include <stdint.h>

int cp_socket_init();
void cp_socket_uninit();

bool cp_socket_valid(cp_socket_t sock);
cp_socket_t cp_socket(int af, int type, int protocol);
cp_socket_t cp_socket_udp(int af = AF_INET, int type = SOCK_DGRAM, int protocol = IPPROTO_UDP);
cp_socket_t cp_socket_tcp(int af = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
int cp_bind(cp_socket_t sock, int af, const char *p_srv_host, uint16_t srv_port);
int cp_connect(cp_socket_t sock, int af, const char *p_srv_host, uint16_t srv_port);
// int cp_bind(cp_socket_t sock, uint16_t srv_port, const char *p_srv_host = NULL, int af = AF_INET);
int cp_listen(cp_socket_t sock, int maxconn);
cp_socket_t cp_accept(cp_socket_t sock, struct sockaddr_in *p_addr);
int cp_send(cp_socket_t sock, const char *buf, int len, int flags = 0);
int cp_recv(cp_socket_t sock, void *buf, size_t len, int flags = 0);
int cp_send_all(cp_socket_t sock, const char *buf, int len, int flags = 0);
int cp_setblocking(cp_socket_t sock, bool is_block);
void cp_close_socket(cp_socket_t sock);
cp_socket_t create_connection(const char *server_ip, uint16_t server_port);

const char *cp_inet_ntop(struct sockaddr_in *src, char *dst, size_t len);
int cp_inet_pton(int af, const char *p_host, struct sockaddr_in *p_addr);

int cp_getpeername(cp_socket_t sock, struct sockaddr_in *sin);