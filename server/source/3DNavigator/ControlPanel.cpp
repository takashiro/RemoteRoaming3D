
#include "ControlPanel.h"
#include "Server.h"
#include "ServerUser.h"
#include "Hotspot.h"

#include <irrlicht.h>
#include <iostream>

using namespace std;
using namespace irr;

map<string, ControlPanel::Callback> ControlPanel::_callbacks;

#define ADD_CALLBACK(command) _callbacks[#command]=&ControlPanel::_##command

ControlPanel::ControlPanel(istream &in, ostream &out)
	:cin(in), cout(out)
{
	if(_callbacks.empty())
	{
		ADD_CALLBACK(server);
		ADD_CALLBACK(client);
		ADD_CALLBACK(help);
	}
}

void ControlPanel::_help()
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
	map<string,Callback>::iterator func;
	while(true)
	{
		cout << "$ ";
		cin >> cmd;
		if(cmd == "quit")
		{
			return 0;
		}
		
		func = _callbacks.find(cmd);
		if(func != _callbacks.end())
		{
			Callback callback = func->second;
			if(callback != NULL)
				(this->*callback)();
		}
		else
		{
			cout << "invalid command" << endl;
		}
	}

	return 0;
}

void ControlPanel::_server()
{
	string cmd;
	cin >> cmd;

	if(cmd == "info")
	{
		cout << "Server IP: 0.0.0.0" << endl;
		cout << "Server Port: " << ServerInstance->getServerPort() << endl;
		cout << "Client Number: " << ServerInstance->getClients().size() << " / " << ServerInstance->getMaximumClientNum() << endl;
	}
	else if(cmd == "maxclient")
	{
		int number;
		cin >> number;
		ServerInstance->setMaximumClientNum(number);
	}
	else if(cmd == "listen")
	{
		unsigned short port;
		cin >> port;
		ServerInstance->listenTo(port);
		ServerInstance->broadcastConfig();
		cout << "server started" << endl;
	}
	else if(cmd == "start")
	{
		ServerInstance->listenTo();
		ServerInstance->broadcastConfig();
		cout << "server started" << endl;
	}
	else
	{
		cout << "info -- display server configurations" << endl;
		cout <<"maxclient [int] -- set the maximum number of clients" << endl;
	}
}

void show_client_info(ServerUser *user)
{
	cout << "Client IP: " << user->getIp() << endl;
	IrrlichtDevice *device = user->getDevice();
	if (device) {
		scene::ICameraSceneNode *camera = device->getSceneManager()->getActiveCamera();
		if (camera)
		{
			cout.setf(ios::fixed, ios::floatfield);
			cout.precision(3);

			const core::vector3df &pos = camera->getPosition();
			cout << "Camera Position: " << pos.X << ", " << pos.Y << ", " << pos.Z << endl;
			const core::vector3df &target = camera->getTarget();
			cout << "Camera Target: " << target.X << ", " << target.Y << ", " << target.Z << endl;
		}
	}
	else {
		cout << "No Device" << endl;
	}
}

void ControlPanel::_client()
{
	string idstr;
	cin >> idstr;

	const std::list<ServerUser *> &clients = ServerInstance->getClients();

	if(idstr == "all")
	{
		for (ServerUser *client : clients)
		{
			show_client_info(client);
			cout << endl;
		}
	}
	else
	{
		int id = stoi(idstr);
		int cur = 1;
		for (ServerUser *client : clients)
		{
			if(id == cur)
			{
				show_client_info(client);
				return;
			}
			cur++;
		}

		cout << "no client " << id << endl;
	}
}
