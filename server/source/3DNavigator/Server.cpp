#include "Server.h"
#include "ServerUser.h"
#include "Thread.h"

#include <iostream>
#include <fstream>

RD_NAMESPACE_BEGIN

Server::Server()
	: mServerPort(6666)
	, mMaximumClientNum(10)
	, mIsListening(false)
	, mBroadcastSocket(nullptr)
	, mBroadcastPort(52600)
	, mIsBroadcastingConfig(false)
	, mServerThread(nullptr)
	, mBroadcastThread(nullptr)
{
	//create a TCP server
	mServerSocket = new TCPServer;
}

Server::~Server()
{
	if (isListening()) {
		mServerSocket->close();
		mServerThread->wait();
		delete mServerThread;
	}

	if (mBroadcastThread) {
		delete mBroadcastThread;
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
	if (!mServerSocket->listen(IP::AnyHost, port)) {
		return;
	}

	mServerThread = new Thread([this]() {
		for (;;) {
			TCPSocket *client_socket = mServerSocket->nextPendingConnection();
			if (client_socket == nullptr) {
				break;
			}

			ServerUser *client = new ServerUser(this, client_socket);
			if (this->getClients().size() >= this->getMaximumClientNum()) {
				client->makeToast(server_reaches_max_client_num);
				delete client;
			} else {
				client->startService();
				if (client->isValid()) {
					mClients.push_back(client);
				} else {
					delete client;
				}
			}
		}
	});
}

void Server::broadcastConfig(ushort port)
{
	if (port != 0) {
		mBroadcastPort = port;
		mIsBroadcastingConfig = true;
		if (mBroadcastThread == nullptr) {
			mBroadcastThread = new Thread([this]() {
				ushort port = this->getServerPort();
				const char *send_data = reinterpret_cast<const char *>(&port);
				char receive_data[2];

				UDPSocket *&socket = mBroadcastSocket;
				socket = new UDPSocket;
				socket->bind(IP::AnyHost, 5261);

				IP client_ip;
				unsigned short client_port;
				while (mIsBroadcastingConfig) {
					socket->receiveFrom(receive_data, 2, client_ip, client_port);
					socket->sendTo(send_data, 2, client_ip, 5260);
				}
			});
		}
	} else {
		mIsBroadcastingConfig = false;
		if (mBroadcastThread) {
			mBroadcastThread->wait();
			delete mBroadcastThread;
			mBroadcastThread = nullptr;
		}
	}
}

void Server::remove(ServerUser *client)
{
	mClients.remove(client);
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
