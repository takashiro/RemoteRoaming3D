
#include "ControlPanel.h"
#include "Server.h"
#include "ServerUser.h"
#include "Hotspot.h"

#include <irrlicht.h>
#include <iostream>

using namespace std;
using namespace irr;

RD_NAMESPACE_BEGIN

map<string, ControlPanel::Callback> ControlPanel::mCallbacks;

#define ADD_CALLBACK(command) mCallbacks[#command]=&ControlPanel::##command

ControlPanel::ControlPanel(istream &in, ostream &out)
	: cin(in)
	, cout(out)
	, mServer(nullptr)
{
	if (mCallbacks.empty()) {
		ADD_CALLBACK(server);
		ADD_CALLBACK(client);
		ADD_CALLBACK(help);
	}
}

ControlPanel::~ControlPanel()
{
	if (mServer) {
		delete mServer;
	}
}

void ControlPanel::help()
{
	cout << "server  -- to configure the server" << endl;
	cout << "client  -- control clients" << endl;
	cout << "help  -- provides help information" << endl;
	cout << "quit  -- to quit the server" << endl;
}

int ControlPanel::exec()
{
	cout << "Welcome to Remote Roaming 3D Server!" << endl;

	string cmd;
	map<string, Callback>::iterator func;
	forever{
		cout << "$ ";
		cin >> cmd;
		if (cmd == "quit") {
			return 0;
		}

		func = mCallbacks.find(cmd);
		if (func != mCallbacks.end()) {
			Callback callback = func->second;
			if (callback) {
				(this->*callback)();
			}
		} else {
			cout << "invalid command" << endl;
		}
	}

	return 0;
}

void ControlPanel::server()
{
	string cmd;
	cin >> cmd;

	if (cmd == "info") {
		if (mServer) {
			cout << "Server IP: 0.0.0.0" << endl;
			cout << "Server Port: " << mServer->getServerPort() << endl;
			cout << "Client Number: " << mServer->getClients().size() << " / " << mServer->getMaximumClientNum() << endl;
		} else {
			cout << "Server is not running" << endl;
		}
	} else if (cmd == "maxclient") {
		if (mServer) {
			int number;
			cin >> number;
			mServer->setMaximumClientNum(number);
		} else {
			cout << "Server is not running" << endl;
		}
	} else if (cmd == "listen") {
		if (mServer) {
			unsigned short port;
			cin >> port;
			mServer->listenTo(port);
			mServer->broadcastConfig();
			cout << "server started" << endl;
		} else {
			cout << "Server is not running" << endl;
		}
	} else if (cmd == "start") {
		if (mServer == nullptr) {
			mServer = new Server;
			mServer->setDriverType(irr::video::EDT_OPENGL);
			mServer->loadSceneMap("scenemap.json");
			mServer->listenTo();
			mServer->broadcastConfig();
			cout << "Server started" << endl;
		} else {
			cout << "Server has already started" << endl;
		}
	} else {
		cout << "info -- display server configurations" << endl;
		cout << "maxclient [int] -- set the maximum number of clients" << endl;
	}
}

void show_client_info(ServerUser *user)
{
	cout << "Client IP: " << user->getIp() << endl;
	IrrlichtDevice *device = user->getDevice();
	if (device) {
		scene::ICameraSceneNode *camera = device->getSceneManager()->getActiveCamera();
		if (camera) {
			cout.setf(ios::fixed, ios::floatfield);
			cout.precision(3);

			const core::vector3df &pos = camera->getPosition();
			cout << "Camera Position: " << pos.X << ", " << pos.Y << ", " << pos.Z << endl;
			const core::vector3df &target = camera->getTarget();
			cout << "Camera Target: " << target.X << ", " << target.Y << ", " << target.Z << endl;
		}
	} else {
		cout << "No Device" << endl;
	}
}

void ControlPanel::client()
{
	string idstr;
	cin >> idstr;

	if (mServer == nullptr) {
		cout << "Server is not running" << endl;
	}

	const std::list<ServerUser *> &clients = mServer->getClients();

	if (idstr == "all") {
		for (ServerUser *client : clients) {
			show_client_info(client);
			cout << endl;
		}
	} else {
		int id = stoi(idstr);
		int cur = 1;
		for (ServerUser *client : clients) {
			if (id == cur) {
				show_client_info(client);
				return;
			}
			cur++;
		}

		cout << "no client " << id << endl;
	}
}

RD_NAMESPACE_END
