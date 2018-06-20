#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;


#define data_len_t uint32_t

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	int32_t		id;
	data_len_t	length;
}Header;

typedef struct
{
	Header	header;
	std::shared_ptr<void> data;
}Request;

typedef struct
{
	Header	header;
	int32_t	errcode;
}Response;
#pragma pack(pop)


int loop_send_recv(int argc, const char **argv)
{
//	const char *srv_host = "192.168.6.174";
	const char *srv_host = "127.0.0.1";
	uint16_t srv_port = 7890;

	//创建客户端套接字
	while (true)
	{
		// socket
		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sclient == INVALID_SOCKET)
		{
			printf("invalid socket!");
			goto loop_send_recv_end;
		}
		printf("client socket success\n");

		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(srv_port);
		inet_pton(AF_INET, srv_host, (void *)&serAddr.sin_addr.S_un.S_addr);

		// connect
		if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{
			printf("connect error !");
			closesocket(sclient);
			goto loop_send_recv_end;
		}
		printf("client connect to %s:%d success\n", srv_host, srv_port);

		string send_buff("sssssssssss");
		printf("input (use `quit` to quit..): \n");
		cin >> send_buff;

		if (send_buff.compare(0, 4, "quit") == 0)
		{
			printf("quit this conversation..\n");
			closesocket(sclient);
			goto loop_send_recv_end;
		}

		send(sclient, send_buff.data(), (int)send_buff.size(), 0);
		printf("send: [%s]\n", send_buff.data());


		char recv_buff[255];
		int n_recv = recv(sclient, recv_buff, 255, 0);
		if (n_recv == SOCKET_ERROR)
		{
			printf("recv occur an error\n");
			break;
		}
		else if (n_recv == 0)
		{
			printf("this socket(%d) is normally closed\n", n_recv);
			break;
		}
		else
		{
			if (n_recv > sizeof(recv_buff) - 1)
			{
				n_recv = sizeof(recv_buff) - 1;
			}
			recv_buff[n_recv] = 0x00;
			printf("recv: [%s]\n", recv_buff);
		}
		closesocket(sclient);
//		Sleep(100);
	}


loop_send_recv_end:
	printf("input a enter to quit..");
	getchar();
	return 0;
}


int fake_client_send_file(int argc, const char **argv)
{

	//	const char *srv_host = "192.168.6.174";
	const char *srv_host = "127.0.0.1";
	uint16_t srv_port = 7890;

	int n_send = 0, n_recv = 0;
	
	Request req;
	Response resp;

	std::ifstream is(argv[0], std::ifstream::binary);
	if (!is)
	{
		printf("open(%s) failed\n", argv[0]);
		return -1;
	}

	// socket
	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket!");
		return -1;
	}
	printf("client socket success\n");

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(srv_port);
	inet_pton(AF_INET, srv_host, (void *)&serAddr.sin_addr.S_un.S_addr);

	// connect
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		goto loop_send_recv_end;
	}
	printf("client connect to %s:%d success\n", srv_host, srv_port);

	// alloc buffer
	is.seekg(0, is.end);
	req.header.length = (int)is.tellg();
	is.seekg(0, is.beg);

	req.data = std::shared_ptr<char>(new char[req.header.length]);
	if (req.data.get() == nullptr)
	{
		printf("alloc(%d) failed\n", req.header.length);
		goto loop_send_recv_end;
	}

	// read file
	is.read((char *)req.data.get(), req.header.length);
	printf("read file(%s) %d/%dB\n", argv[0], (int)is.gcount(), req.header.length);
	if (!is)
	{
		goto loop_send_recv_end;
	}

	// send header
	req.header.id = htonl(9999);
	n_send = send(sclient, (const char *)&req.header, (int)sizeof(req.header), 0);
	if (n_send == SOCKET_ERROR)
	{
		printf("recv occur an error send:%d\n", n_send);
		goto loop_send_recv_end;
	}
	else if (n_send == 0)
	{
		printf("recv occur an error send:%d\n", n_send);
		goto loop_send_recv_end;
	}
	printf("send head %dB, id=%d, vlen=%d\n", n_send, req.header.id, req.header.length);

	// send buffer
	n_send = send(sclient, (const char *)req.data.get(), (int)req.header.length, 0);
	if (n_send == SOCKET_ERROR)
	{
		printf("recv occur an error send:%d\n", n_send);
		goto loop_send_recv_end;
	}
	else if (n_send == 0)
	{
		printf("recv occur an error send:%d\n", n_send);
		goto loop_send_recv_end;
	}
	printf("send %dB \n", n_send);

	// wait peer response
	Sleep(100);

	// recv response
	n_recv = recv(sclient, (char *)&resp, sizeof(resp), 0);
	if (n_recv == SOCKET_ERROR)
	{
		printf("recv occur an error recv:%d\n", n_recv);
		goto loop_send_recv_end;
	}
	else if (n_recv == 0)
	{
		printf("this socket(%d) is normally closed\n", n_recv);
		goto loop_send_recv_end;
	}
	if (n_recv != sizeof(resp))
	{
		printf("recv occur an error recv:%d/%dB\n", n_recv, (int)sizeof(resp));
		goto loop_send_recv_end;
	}
	resp.header.id = ntohl(resp.header.id);
	resp.header.length = ntohl(resp.header.length);
	resp.errcode = ntohl(resp.errcode);
	printf("recv: id:%d,vlen:%d,errcode:%d\n", resp.header.id, resp.header.length, resp.errcode);

loop_send_recv_end:
	is.close();
	closesocket(sclient);

	printf("input a enter to quit..");

	getchar();
	return 0;
}

int main(int argc, const char **argv)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA setup_data;
	if (WSAStartup(sockVersion, &setup_data) != 0)
	{
		return 0;
	}

//	loop_send_recv(argc, argv);
	fake_client_send_file(argc, argv);

	WSACleanup();
	return 0;
}