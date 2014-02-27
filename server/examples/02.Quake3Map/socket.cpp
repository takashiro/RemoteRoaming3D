#include "socket.h"
#include <iostream>

SOCKET sock_client;

bool CreateSocket()
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (0 != WSAStartup(sockVersion, &wsaData))
	{
		return 0;
	}
	SOCKET sock_sev = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (INVALID_SOCKET == sock_sev)
	{
		WSACleanup();
		return 0;
	}
	sockaddr_in addr_sev;
	addr_sev.sin_family = AF_INET;
	addr_sev.sin_port = htons(PORT_SERVER);
	addr_sev.sin_addr.s_addr = INADDR_ANY;
	if (SOCKET_ERROR == bind(sock_sev, (sockaddr *)&addr_sev, sizeof(addr_sev)))
	{
		WSACleanup();
		return 0;
	}
	if (SOCKET_ERROR == listen(sock_sev, NUM_CLIENTS))
	{
		WSACleanup();
		return 0;
	}

	sockaddr_in addr_client;
	int nAddrLen = sizeof(addr_client);
	//while (true)
	//{
		sock_client = accept(sock_sev, (sockaddr *)&addr_client, &nAddrLen);

		if (INVALID_SOCKET == sock_client)
		{
			exit(0);
		}

		printf("connected...................\n");

		//char tmp[20] = "connected";
		//send(sock_client, tmp, strlen(tmp), 0);

		//closesocket(sock_client);
	//}
}
