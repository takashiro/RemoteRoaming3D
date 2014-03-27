#include "Server.h"
#include "ServerUser.h"

Server *ServerInstance = NULL;

Server::Server()
	:_server_port(6666), _maximum_client_num(1), _is_listening(false), _is_independent_thread_enabled(true)
{
	//create a TCP server
	_server_socket = new R3D::TCPServer();
}

Server::~Server()
{
	if(isListening()){
		_server_socket->close();
		CloseHandle(_server_thread);
	}

	for(std::list<ServerUser *>::iterator i = _clients.begin(); i != _clients.end(); i++)
	{
		(*i)->disconnect();
	}

	delete _server_socket;
}

void Server::listenTo(unsigned short port)
{
	if(isListening())
	{
		return;
	}
	_is_listening = true;

	_server_port = port;
	
	//listen to any ip address of the local host
	if (!_server_socket->listen(R3D::IP::AnyHost, port))
	{
		return;
	}

	if(isIndependentThreadEnabled()){
		_server_thread = CreateThread(NULL, 0, _ServerThread, (LPVOID) this, 0, NULL);
	}else{
		_ServerThread((LPVOID) this);
	}
}

DWORD WINAPI Server::_ServerThread(LPVOID pParam)
{
	Server *server = (Server *) pParam;

	R3D::TCPServer *&server_socket = server->_server_socket;
	
	while(true)
	{
		R3D::TCPSocket *client_socket = server_socket->nextPendingConnection();
		if (client_socket == NULL)
		{
			continue;
		}

		ServerUser *client = new ServerUser(server, client_socket);
		if(server->getClients().size() >= server->getMaximumClientNum()){
			R3D::Packet packet(R3D::MakeToastText);
			packet.args[0] = (int) R3D::server_reaches_max_client_num;
			client->sendPacket(packet);
			delete client;
		}else{
			client->startService();
			if(client->isValid()){
				server->_clients.push_back(client);
			}else{
				delete client;
			}
		}

		Sleep(100);
	}

	return 0;
}

void Server::disconnect(ServerUser *client){
	_clients.remove(client);
	client->disconnect();
}
