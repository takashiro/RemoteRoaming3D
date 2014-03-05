#include "Server.h"
#include "ServerUser.h"

inline void printip(const sockaddr_in &ip){
	printf("%d.%d.%d.%d", ip.sin_addr.S_un.S_un_b.s_b1, ip.sin_addr.S_un.S_un_b.s_b2, ip.sin_addr.S_un.S_un_b.s_b3, ip.sin_addr.S_un.S_un_b.s_b4);
}

Server::Server()
	:_server_port(6666), _maximum_client_num(1), _is_listening(false), _is_independent_thread_enabled(false)
{
}

Server::~Server()
{
	if(isListening()){
		closesocket(_server_socket);
		CloseHandle(_server_thread);
	}

	for(std::list<ServerUser *>::iterator i = _clients.begin(); i != _clients.end(); i++){
		delete *i;
	}
}

void Server::listenTo(short port)
{
	if(isListening())
	{
		return;
	}
	_is_listening = true;

	_server_port = port;

	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData))
	{
		return;
	}
	
	//try to create a TCP socket
	SOCKET sock_sev = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock_sev)
	{
		WSACleanup();
		return;
	}
	
	//listen to any ip address of the local host
	sockaddr_in addr_sev;
	addr_sev.sin_family = AF_INET;
	addr_sev.sin_port = htons(_server_port);
	addr_sev.sin_addr.s_addr = INADDR_ANY;
	if (SOCKET_ERROR == bind(sock_sev, (sockaddr *)&addr_sev, sizeof(addr_sev)))
	{
		WSACleanup();
		return;
	}
	if (SOCKET_ERROR == listen(sock_sev, _maximum_client_num))
	{
		WSACleanup();
		return;
	}

	_server_socket = sock_sev;

	if(isIndependentThreadEnabled()){
		_server_thread = CreateThread(NULL, 0, _ServerThread, (LPVOID) this, 0, NULL);
	}else{
		_ServerThread((LPVOID) this);
	}
}

DWORD WINAPI Server::_ServerThread(LPVOID pParam)
{
	Server *server = (Server *) pParam;

	sockaddr_in addr_client;
	int nAddrLen = sizeof(addr_client);

	SOCKET server_socket = (SOCKET) server->_server_socket;
	
	while(true)
	{
		SOCKET client_socket = accept(server_socket, (sockaddr *)&addr_client, &nAddrLen);
		if (client_socket == INVALID_SOCKET)
		{
			continue;
		}

		//@to-do:refuse new connections if the server reaches the maximum client number

		ServerUser *client = new ServerUser(server, client_socket);
		if(client->isValid()){
			server->_clients.push_back(client);
			printf("new client:");
			printip(addr_client);
			printf("\n");
		}else{
			delete client;
		}
	}

	return 0;
}

void Server::disconnect(ServerUser *client){
	for(std::list<ServerUser *>::iterator i = _clients.begin(); i != _clients.end(); i++){
		if(*i == client){
			_clients.remove(client);
			printf("client disconnected:");
			printip(client->getIp());
			printf("\n");
			break;
		}
	}
}
