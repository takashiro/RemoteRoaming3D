
#include "cpanel.h"
#include "Server.h"
#include "ServerUser.h"

#include <irrlicht.h>
#include <iostream>

using namespace std;
using namespace irr;

map<string, CPanelCallback> CPanelCommand;

void server()
{
	printf("Server IP: 0.0.0.0\n");
	printf("Server Port: %d\n", ServerInstance->getServerPort());
	printf("Client Number: %d / %d\n", ServerInstance->getClients().size(), ServerInstance->getMaximumClientNum());
}

void show_client_info(ServerUser *user)
{
	sockaddr_in ip = user->getIp();
	printf("Client IP: %d.%d.%d.%d\n", ip.sin_addr.S_un.S_un_b.s_b1, ip.sin_addr.S_un.S_un_b.s_b2, ip.sin_addr.S_un.S_un_b.s_b3, ip.sin_addr.S_un.S_un_b.s_b4);
	IrrlichtDevice *device = user->getDevice();
	scene::ICameraSceneNode *camera = device->getSceneManager()->getActiveCamera();
	if(camera != NULL)
	{
		const core::vector3df &pos = camera->getPosition();
		printf("Camera Position: %.3f, %.3f, %.3f\n", pos.X, pos.Y, pos.Z);
		const core::vector3df &target = camera->getTarget();
		printf("Camera Target: %.3f, %.3f, %.3f\n", target.X, target.Y, target.Z);
	}
}

void client(){
	string idstr;
	cin >> idstr;

	const std::list<ServerUser *> &clients = ServerInstance->getClients();

	if(idstr == "all")
	{
		for(std::list<ServerUser *>::const_iterator i = clients.begin(); i != clients.end(); i++)
		{
			show_client_info(*i);
			printf("\n");
		}
	}
	else
	{
		int id = atoi(idstr.c_str());
		int cur = 1;
		for(std::list<ServerUser *>::const_iterator i = clients.begin(); i != clients.end(); i++)
		{
			if(id == cur)
			{
				show_client_info(*i);
				return;
			}
		}

		printf("no client %d\n", id);
	}
}

class CommandAdder{
public:
	CommandAdder(){
		CPanelCommand["server"] = &server;
		CPanelCommand["client"] = &client;
	}
};

CommandAdder adder;
