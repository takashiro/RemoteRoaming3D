#include "server.h"
#include "serveruser.h"
#include "protocol.h"

#include <memory.h>
#include <iostream>
#include <irrlicht.h>

using namespace irr;

ServerUser::ServerUser(Server *server, SOCKET socket){
	_server = server;
	_socket = socket;
	_receive_thread = CreateThread(NULL, 0, _ReceiveThread, (LPVOID) this, 0, NULL);
}

ServerUser::~ServerUser(){
	closesocket(_socket);

	if(_receive_thread != NULL){
		CloseHandle(_receive_thread);
	}
}

DWORD WINAPI ServerUser::_ReceiveThread(LPVOID pParam){
	ServerUser *client = (ServerUser *) pParam;
	SOCKET socket = client->_socket;

	client->sendScreenshot();

	int result = 0;
	const int length = 1024;
	char buffer[length];

	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		result = recv(socket, buffer, length, 0);
		
		if (result == 0 || result == SOCKET_ERROR) 
		{
			//disconnect the client
			client->_server->disconnect(client);
			break;
		}

		client->handleCommand(buffer);
	}

	return 0;
}

void ServerUser::sendScreenshot(){
	IrrlichtDevice *device = _server->getIrrlichtDevice();
	video::IImage* image = device->getVideoDriver()->createScreenShot();
	if (image)
	{
		device->getVideoDriver()->writeImageToFile(image, "screenshot.jpg");
		image->drop();
	}

	const int SIZE = 1024 * 8;
	FILE* fp;
	int send_count;
	int length;

	if((fp=fopen("screenshot.jpg","rb"))==NULL)
	{
		printf("从服务器端返回文件未打开\n");
	}

	fseek(fp,0L,SEEK_END);
	length=ftell(fp);

	send(_socket, (char *)&length + 3, 1, 0);
	send(_socket, (char *)&length + 2, 1, 0);
	send(_socket, (char *)&length + 1, 1, 0);
	send(_socket, (char *)&length + 0, 1, 0);
	fseek(fp, 0L, SEEK_SET);
		
	//transfer screenshot
	long int y=0;
	char trans[SIZE];
	while(!feof(fp))
	{
		fread(trans,1,SIZE,fp);
		y=y+SIZE;
		if(y<length)
		{
			send_count=send(_socket, trans, SIZE, 0);
		}
		else
		{
			send(_socket, trans ,length + SIZE - y, 0);
		}
	}
	fclose(fp);
}

void ServerUser::handleCommand(const char *cmd)
{
	R3D::Packet packet = R3D::Packet::FromString(cmd);

	IrrlichtDevice *device = _server->getIrrlichtDevice();
	HWND HWnd = GetFocus();//@to-do:It was the HWND inside IrrlichtDevice

	int x1 = packet.args[0].asInt();
	int y1 = packet.args[1].asInt();
	
	if(packet.command == R3D::Packet::Move)
	{
		int x = device->getCursorControl()->getPosition().X - x1 * 10;
		int y = device->getCursorControl()->getPosition().Y - y1 * 10;
		device->getCursorControl()->setPosition(x,y);

		PostMessage(HWnd, WM_MOUSEMOVE, 0, MAKELONG(x,y));
	}
	else
	{
		if(x1 < 0)
		{
			PostMessage(HWnd,WM_KEYDOWN, VK_UP, 0);
			Sleep(100);
			PostMessage(HWnd,WM_KEYUP, VK_UP, 0);
		}
		else
		{
			PostMessage(HWnd,WM_KEYDOWN, VK_DOWN, 0);
			Sleep(100);
			PostMessage(HWnd,WM_KEYUP, VK_DOWN, 0);
		}
	}

	sendScreenshot();
}
