#include "Server.h"
#include "ServerUser.h"

#include <iostream>
#include <fstream>

RD_NAMESPACE_BEGIN

Server::Server()
	:mServerPort(6666), mMaximumClientNum(10), mIsListening(false), mIsIndependentThreadEnabled(true),
	mBroadcastSocket(nullptr), mBroadcastThread(nullptr), mBroadcastPort(52600), mIsBroadcastingConfig(false)
{
	//create a TCP server
	mServerSocket = new R3D::TCPServer;
}

Server::~Server()
{
	if (isListening()) {
		mServerSocket->close();
		CloseHandle(mServerThread);
	}

	if (mBroadcastThread) {
		CloseHandle(mBroadcastThread);
	}

	for (ServerUser *client : mClients) {
		client->disconnect();
	}

	for (SceneMap *map : mScenemaps) {
		delete map;
	}

	delete mServerSocket;
}

void Server::listenTo(unsigned short port)
{
	if (isListening()) {
		return;
	}
	mIsListening = true;

	mServerPort = port;

	//listen to any ip address of the local host
	if (!mServerSocket->listen(R3D::IP::AnyHost, port)) {
		return;
	}

	if (isIndependentThreadEnabled()) {
		mServerThread = CreateThread(NULL, 0, ServerThread, (LPVOID) this, 0, NULL);
	} else {
		ServerThread((LPVOID) this);
	}
}

DWORD WINAPI Server::ServerThread(LPVOID pParam)
{
	Server *server = (Server *)pParam;

	R3D::TCPServer *&server_socket = server->mServerSocket;

	while (true) {
		R3D::TCPSocket *client_socket = server_socket->nextPendingConnection();
		if (client_socket == nullptr) {
			continue;
		}

		ServerUser *client = new ServerUser(server, client_socket);
		if (server->getClients().size() >= server->getMaximumClientNum()) {
			client->makeToast(R3D::server_reaches_max_client_num);
			delete client;
		} else {
			client->startService();
			if (client->isValid()) {
				server->mClients.push_back(client);
			} else {
				delete client;
			}
		}
	}

	return 0;
}

void Server::broadcastConfig(ushort port)
{
	if (port != 0) {
		mBroadcastPort = port;
		mIsBroadcastingConfig = true;
		if (mBroadcastThread == nullptr) {
			mBroadcastThread = CreateThread(NULL, 0, BroadcastThread, (LPVOID) this, 0, NULL);
		}
	} else {
		mIsBroadcastingConfig = false;
		if (mBroadcastThread) {
			WaitForSingleObject(mBroadcastThread, INFINITE);
			CloseHandle(mBroadcastThread);
			mBroadcastThread = nullptr;
		}
	}
}

DWORD WINAPI Server::BroadcastThread(LPVOID pParam)
{
	Server *server = (Server *)pParam;
	ushort port = server->getServerPort();
	const char *send_data = reinterpret_cast<const char *>(&port);
	char receive_data[2];

	R3D::UDPSocket *&socket = server->mBroadcastSocket;
	socket = new R3D::UDPSocket;
	socket->bind(R3D::IP::AnyHost, 5261);

	R3D::IP client_ip;
	unsigned short client_port;
	while (server->mIsBroadcastingConfig) {
		socket->receiveFrom(receive_data, 2, client_ip, client_port);
		socket->sendTo(send_data, 2, client_ip, 5260);
	}

	return 0;
}

void Server::disconnect(ServerUser *client)
{
	mClients.remove(client);
	client->disconnect();
}

void Server::loadSceneMap(const std::string &config_path)
{
	std::ifstream config(config_path, std::ios::binary);
	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(config, value))
		return;

	for (const Json::Value &i : value) {
		mScenemaps.push_back(new SceneMap(i));
	}
}

RD_NAMESPACE_END
