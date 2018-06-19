#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstring>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	//初始化WSA windows自带的socket
	{
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA setup_data;
		if (WSAStartup(sockVersion, &setup_data) != 0)
		{
			return 0;
		}
	}

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
			goto main_end;
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
			goto main_end;
		}
		printf("client connect to %s:%d success\n", srv_host, srv_port);

		string send_buff("sssssssssss");
		printf("input (use `quit` to quit..): \n");
		cin >> send_buff;

		if (send_buff.compare(0, 4, "quit") == 0)
		{
			printf("quit this conversation..\n");
			closesocket(sclient);
			goto main_end;
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
	WSACleanup();

main_end:
	printf("input a enter to quit..");
	getchar();
	return 0;
}