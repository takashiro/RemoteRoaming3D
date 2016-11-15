#include "Server.h"
#include "ServerUser.h"

#include <iostream>
#include <fstream>

Server *ServerInstance = NULL;

Server::Server()
	:_server_port(6666), _maximum_client_num(10), _is_listening(false), _is_independent_thread_enabled(true),
	_broadcast_socket(NULL), _broadcast_thread(NULL), _broadcast_port(52600), _is_broadcasting_config(false)
{
	ServerInstance = this;

	//create a TCP server
	_server_socket = new R3D::TCPServer;
}

Server::~Server()
{
	if(isListening()){
		_server_socket->close();
		CloseHandle(_server_thread);
	}

	if(_broadcast_thread != NULL){
		CloseHandle(_broadcast_thread);
	}

	for(std::list<ServerUser *>::iterator i = _clients.begin(); i != _clients.end(); i++)
	{
		(*i)->disconnect();
	}

	for(std::vector<SceneMap *>::iterator i = _scenemaps.begin(); i != _scenemaps.end(); i++)
	{
		delete *i;
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
			client->makeToast(R3D::server_reaches_max_client_num);
			delete client;
		}else{
			client->startService();
			if(client->isValid()){
				server->_clients.push_back(client);
			}else{
				delete client;
			}
		}
	}

	return 0;
}

void Server::broadcastConfig(unsigned short port)
{
	if(port != 0)
	{
		_broadcast_port = port;
		_is_broadcasting_config = true;
		if(_broadcast_thread == NULL)
		{
			_broadcast_thread = CreateThread(NULL, 0, _BroadcastThread, (LPVOID) this, 0, NULL);
		}
	}
	else
	{
		_is_broadcasting_config = false;
		if(_broadcast_thread != NULL)
		{
			WaitForSingleObject(_broadcast_thread, INFINITE);
			CloseHandle(_broadcast_thread);
			_broadcast_thread = NULL;
		}
	}
}

DWORD WINAPI Server::_BroadcastThread(LPVOID pParam)
{
	Server *server = (Server *) pParam;
	unsigned short port = server->getServerPort();
	const char *send_data = (const char *) &port;
	char receive_data[2];

	R3D::UDPSocket *&socket = server->_broadcast_socket;
	socket = new R3D::UDPSocket;
	socket->bind(R3D::IP::AnyHost, 5261);

	R3D::IP client_ip;
	unsigned short client_port;
	while(server->_is_broadcasting_config)
	{
		socket->receiveFrom(receive_data, 2, client_ip, client_port);
		socket->sendTo(send_data, 2, client_ip, 5260);
	}

	return 0;
}

void Server::disconnect(ServerUser *client)
{
	_clients.remove(client);
	client->disconnect();
}

void Server::loadSceneMap(const std::string &config_path)
{
	std::ifstream config(config_path, std::ios::binary);
	Json::Value value;
	Json::Reader reader;
	if(!reader.parse(config, value))
		return;

	for(Json::Value::iterator i = value.begin(); i != value.end(); i++)
	{
		_scenemaps.push_back(new SceneMap(*i));
	}
}
