#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <WinSock.h>
#pragma comment(lib,"ws2_32.lib")

#define PORT_SERVER 6666
#define NUM_CLIENTS 10

extern SOCKET sock_client;

bool CreateSocket();
SOCKET GetUsingSocket();

#endif
