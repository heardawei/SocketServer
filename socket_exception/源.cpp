#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <exception>
#include <iostream>
#include <string>

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

#define DEBUG

#ifdef DEBUG

// #define DEBUG_PRINT(fmt, args...)\
//     do {\
//         fprintf(stderr, "DEBUG :%s:%d:%s(): " fmt,\
//                 __FILE__, __LINE__, __func__, ##args);\
//     } while(0)

#define debug_print(...) do{ fprintf(stderr, __VA_ARGS__); }while(0)

#else /* DEBUG */

// #define DEBUG_PRINT(fmt, args...) do{ }while(0)

#define debug_print(...) do{ }while(0)

#endif /* DEBUG */

#include <stdint.h>

#ifdef CP_WINDOWS
#pragma comment(lib, "Ws2_32.lib")
// #pragma comment(lib, "Ntdll.dll")
#endif /* CP_WINDOWS */


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
int cp_recv(cp_socket_t sock, char *buf, int len, int flags = 0);
int cp_send_all(cp_socket_t sock, const char *buf, int len, int flags = 0);
int cp_setblocking(cp_socket_t sock, bool is_block);
void cp_close_socket(cp_socket_t sock);
cp_socket_t create_connection(const char *server_ip, uint16_t server_port);

const char *cp_inet_ntop(struct sockaddr_in *src, char *dst, size_t len);
int cp_inet_pton(int af, const char *p_host, struct sockaddr_in *p_addr);

int cp_getpeername(cp_socket_t sock, struct sockaddr_in *sin);

#ifdef CP_WINDOWS
std::string wstr_2_str(const wchar_t *cws)
{
	// curLocale="C"
	//	string curLocale = setlocale(LC_ALL, NULL);
	//	setlocale(LC_ALL, "chs");
	if (cws == NULL)
	{
		return std::string();
	}
	std::wstring ws = cws;
	size_t convertedChars = 0;

	const wchar_t *wcs = ws.c_str();
	size_t dByteNum = sizeof(wchar_t) * ws.size() + 1;
	// std::cout << "ws.size():" << ws.size() << std::endl;

	char* dest = new char[dByteNum];
	wcstombs_s(&convertedChars, dest, dByteNum, wcs, _TRUNCATE);
	// std::cout << "convertedChars:" << convertedChars << std::endl;

	std::string result = dest;

	delete[] dest;

	//	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

std::wstring str_2_wstr(const char *cstr)
{
	if (cstr == NULL)
	{
		return std::wstring();
	}
	std::string s = cstr;
	size_t convertedChars = 0;
	const char* source = s.c_str();
	size_t charNum = sizeof(char) * s.size() + 1;

	wchar_t* dest = new wchar_t[charNum];
	mbstowcs_s(&convertedChars, dest, charNum, source, _TRUNCATE);

	std::wstring result = dest;

	delete[] dest;
	return result;
}
#endif /* CP_WINDOWS */

int cp_socket_init()
{
#ifdef CP_WINDOWS
	WORD wd = 0;
	WSADATA ws, *p_ws = NULL;
	if (wd == 0)
	{
		wd = MAKEWORD(2, 2);
	}
	if (p_ws == NULL)
	{
		p_ws = &ws;
	}
	int ret = WSAStartup(wd, p_ws);
	if (ret != 0)
	{
		debug_print("WSAStartup failed, return val: %d\n", ret);
		return ret;
	}
#endif /* CP_WINDOWS */
	return 0;
}

void cp_socket_uninit()
{
#ifdef CP_WINDOWS
	WSACleanup();
#endif /* CP_WINDOWS */
}

/* cp is short for cross-platform
* cp_xxx() is package for xxx() with cross-platform
* enjoy !
*/

bool cp_socket_valid(cp_socket_t sock)
{
	return (sock != 0) && (sock != INVALID_SOCKET);
}

cp_socket_t cp_socket(int af, int type, int protocol)
{
	return socket(AF_INET, type, protocol);
}

cp_socket_t cp_socket_udp(int af, int type, int protocol)
{
	return cp_socket(af, type, protocol);
}

cp_socket_t cp_socket_tcp(int af, int type, int protocol)
{
	return cp_socket(af, type, protocol);
}

const char *cp_inet_ntop(struct sockaddr_in *src, char *dst, size_t len)
{
	if (dst == NULL || src == NULL)
	{
		return NULL;
	}
	void *p_s_addr = NULL;
#ifdef CP_WINDOWS
#define ADDR_MAX_SIZE 64
	p_s_addr = (void *)&src->sin_addr.S_un.S_addr;
	wchar_t wstr[ADDR_MAX_SIZE] = { 0 };
	// RtlIpv4AddressToString();
	/*	if (InetNtop(src->sin_family, p_s_addr, wstr, sizeof(wstr)) != NULL)
	{
	std::string str = wstr_2_str(wstr);
	strncpy_s(dst, len, str.data(), str.size());
	return dst;
	}
	return NULL;
	*/
	// return InetNtopA(src->sin_family, p_s_addr, dst, len);

	/* Windows XP doesn't support either inet_ntop or InetNtop*/
	strncpy_s(dst, len, inet_ntoa(src->sin_addr), len);
	return dst;
#elif CP_LINUX
	p_s_addr = (void *)&src->sin_addr.s_addr;
	return inet_ntop(src->sin_family, p_s_addr, dst, len);
#endif /* CP_WINDOWS/CP_LINUX */
}

int cp_inet_pton(int af, const char *p_host, struct sockaddr_in *p_addr)
{
	int ret = 0;

#ifdef CP_WINDOWS
	std::wstring wstr = str_2_wstr(p_host);
	// ret = InetPton(AF_INET, wstr.data(), (void *)&p_addr->sin_addr.S_un.S_addr);
	// ret = InetPtonA(AF_INET, p_host, (void *)&p_addr->sin_addr.S_un.S_addr);
	// ret = RtlIpv4StringToAddressW(wstr.data(), true, NULL, &p_addr->sin_addr);

	/* Windows XP doesn't support either inet_pton or InetPton*/
	if (p_host == NULL || strcmp(p_host, "0.0.0.0") == 0)
	{
		p_addr->sin_addr.S_un.S_addr = INADDR_ANY;
		ret = 1;
	}
	else
	{
		p_addr->sin_addr.S_un.S_addr = inet_addr(p_host);

		// ensure un-formated string return INADDR_NONE
		//	in Windows XP, if p_host is an '\0', INADDR_ANY is returned
		//	so ingored INADDR_ANY
		if (p_addr->sin_addr.S_un.S_addr == INADDR_ANY)
		{
			p_addr->sin_addr.S_un.S_addr = INADDR_NONE;
			ret = 0;
		}
		else if (p_addr->sin_addr.S_un.S_addr == INADDR_NONE)
		{
			ret = -1;
		}
		else
		{
			ret = 1;
		}
	}
#elif CP_LINUX
	if (p_host == NULL || strcmp(p_host, "0.0.0.0") == 0)
	{
		p_addr->sin_addr.s_addr = INADDR_ANY;
		ret = 1;
	}
	else
	{
		ret = inet_pton(AF_INET, p_host, (void *)&p_addr->sin_addr.s_addr);
	}
#endif /* CP_WINDOWS/CP_LINUX */
	return ret;
}

/*
*
* p_srv_host:	1. default is NULL, equal to INADDR_ANY;
* 				2. if p_srv_host is not NULL, bind this specific ethernet
*
* srv_port:	must sepcific a port
*
* ret:			1. if p_srv_host is not NULL && inet_pton failed, -2 is returned
*				2. else returns bind();
*/
int cp_bind(cp_socket_t sock, int af, const char *p_srv_host, uint16_t srv_port)
{
	struct sockaddr_in sin;
	sin.sin_family = af;
	sin.sin_port = htons(srv_port);

	int ret = cp_inet_pton(af, p_srv_host, &sin);
	if (ret == 0 || ret == -1)
	{
		debug_print("cp_inet_pton(%s) failed ret:%d\n", p_srv_host ? p_srv_host : "NULL", ret);
		return -2;
	}
	debug_print("cp_inet_pton(%s) success ret:%d\n", p_srv_host ? p_srv_host : "NULL", ret);

	return bind(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
}

int cp_listen(cp_socket_t sock, int maxconn)
{
	return listen(sock, maxconn);
}

int cp_connect(cp_socket_t sock, int af, const char *p_srv_host, uint16_t srv_port)
{
	struct sockaddr_in sin;
	sin.sin_family = af;
	sin.sin_port = htons(srv_port);

	if (p_srv_host == NULL || srv_port == 0)
	{
		return -2;
	}
	int ret = cp_inet_pton(af, p_srv_host, &sin);
	if (ret == 0 || ret == -1)
	{
		debug_print("cp_inet_pton(%s) failed ret:%d\n", p_srv_host ? p_srv_host : "NULL", ret);
		return -3;
	}
	debug_print("cp_inet_pton(%s) success ret:%d\n", p_srv_host ? p_srv_host : "NULL", ret);

	return connect(sock, (struct sockaddr *)&sin, sizeof(sin));
}

cp_socket_t cp_accept(cp_socket_t sock, struct sockaddr_in *p_addr)
{
	socklen_t cliaddr_len = sizeof(struct sockaddr_in);
	return accept(sock, (struct sockaddr *)p_addr, &cliaddr_len);
}

int cp_send(cp_socket_t sock, const char *buf, int len, int flags)
{
	if (buf == NULL || len < 1)
	{
		return 0;
	}

	return (int)send(sock, buf, len, flags);
}

int cp_recv(cp_socket_t sock, char *buf, int len, int flags)
{
	if (buf == NULL || len < 1)
	{
		return 0;
	}

	return (int)recv(sock, buf, len, flags);
}

int cp_send_all(cp_socket_t sock, const char *buf, int len, int flags)
{
	if (buf == NULL || len < 1)
	{
		return 0;
	}

	int err = len;
	int n_total_send = 0;
	do
	{
		int n_curr_send = send(sock, buf + n_total_send, len - n_total_send, flags);
		if (n_curr_send > 0)
		{
			n_total_send += n_curr_send;
		}
		else if (n_curr_send == 0)
		{
			err = 0;
			break;
		}
		else
		{
			err = -1;
			break;
		}
	} while (n_total_send < len);

	return err;
}

// don't use this un-sound function
static void cp_clear_recv_buffer(cp_socket_t sock)
{
	char tmp[100];
	do
	{
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
#ifdef CP_WINDOWS
		int ret = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
#elif CP_LINUX
		int ret = select(sock + 1, &fds, NULL, NULL, &timeout);
#endif /* CP_WINDOWS/CP_LINUX */
		if (ret == 0)
		{
			break;
		}
		else if (ret == SOCKET_ERROR)
		{
			break;
		}
		if (FD_ISSET(sock, &fds))
		{
			ret = recv(sock, tmp, 100, 0);
		}
	} while (true);
}

int cp_setblocking(cp_socket_t sock, bool is_block)
{
#ifdef CP_WINDOWS
	{
		unsigned long mode = is_block ? 0 : 1;
		return ioctlsocket(sock, FIONBIO, &mode);
	}
#elif CP_LINUX
	{
		int mode = fcntl(sock, F_GETFL, 0);
		mode = is_block ? (mode & O_NONBLOCK) : (mode | O_NONBLOCK);
		return fcntl(sock, F_SETFL, mode);
	}
#endif /* CP_WINDOWS/CP_LINUX */
	return SOCKET_ERROR;
}

void cp_close_socket(cp_socket_t sock)
{
#ifdef CP_WINDOWS
	closesocket(sock);
#elif CP_LINUX
	close(sock);
#endif /* CP_WINDOWS/CP_LINUX */
}

cp_socket_t create_connection(const char *server_ip, uint16_t server_port)
{
	cp_socket_t sock = cp_socket_tcp();
	if (sock == INVALID_SOCKET)
	{
		return -1;
	}

	struct sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip, (void *)&srv_addr.sin_addr);

	if (connect(sock, (sockaddr *)&srv_addr, sizeof(srv_addr)) == SOCKET_ERROR)
	{
		cp_close_socket(sock);
		return -1;
	}
	return sock;
}

int cp_getpeername(cp_socket_t sock, struct sockaddr_in *sin)
{
	size_t len = sizeof(struct sockaddr_in);
#ifdef CP_WINDOWS
	return getpeername(sock, (struct sockaddr *)sin, (int *)&len);
#elif CP_LINUX
	return getpeername(sock, (struct sockaddr *)sin, (socklen_t *)&len);
#endif
}

using namespace std;

int main(void)
{
	cp_socket_t sock = cp_socket_tcp();
	cp_bind(sock, AF_INET, "127.0.0.1", 9956);
	struct sockaddr_in sin;
	try {
		if (cp_getpeername(sock, &sin) == 0)
		{
			char ip[16];
			std::string peer_host = cp_inet_ntop(&sin, ip, sizeof(ip));
			uint16_t peer_port = ntohs(sin.sin_port);
			cout << peer_host << " " << peer_port << endl;
		}
		else
		{
			cout << "catch exception failed, use os error code:" << WSAGetLastError() << endl;
		}
		int a = 0;
		int b = 1;
		int c = b / a;
	}
	catch (exception &e)
	{
		cout << "catch exception success" << e.what() << endl;
	}
	getchar();
	return 0;
}