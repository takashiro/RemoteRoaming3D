#include "server.h"
#include "client.h"

#include <memory.h>
#include <iostream>
#include <irrlicht.h>

using namespace irr;

Client::Client(Server *server, SOCKET socket){
	_server = server;
	_socket = socket;
	_receive_thread = CreateThread(NULL, 0, _ReceiveThread, (LPVOID) this, 0, NULL);
}

Client::~Client(){
	closesocket(_socket);

	if(_receive_thread != NULL){
		CloseHandle(_receive_thread);
	}
}

DWORD WINAPI Client::_ReceiveThread(LPVOID pParam){
	Client *client = (Client *) pParam;
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

void Client::sendScreenshot(){
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

void Client::handleCommand(const char *cmd)
{
	IrrlichtDevice *device = _server->getIrrlichtDevice();
	HWND HWnd = GetFocus();//@to-do:It was the HWND inside IrrlichtDevice

	while(true){
		int flag1;
		int x1;
		int y1;
		char flag[16];
		char xx[16];
		char yy[16];
		char buf[100];
	
		int recv_size = 0;
		
		recv_size = recv(_socket, buf, 100, 0);
		printf("received: %s", buf);
		int i = 0;
		int pos = 0;
		while (buf[i] !=',' && i < 15){
			flag[pos++] = buf[i++];
		}
		flag[pos] = '\0';
		flag1 = atoi(flag);
		i++; 
		pos = 0;
		while (buf[i] !=',' && i < 15)
		{
			xx[pos++] = buf[i++];
		}
		xx[pos] = '\0';
		x1 = atoi(xx);
		i++;
		pos = 0;
		while(buf[i] != '\0' && buf[i] != 'E' && i < 15)
		{
			yy[pos++] = buf[i++];
		}
		yy[pos] = '\0';
		y1 = atoi(yy);

		printf("%d,%d,%d\n",flag1,x1,y1);

		PostMessage(HWnd,WM_KEYDOWN,VK_LEFT,0);
		PostMessage(HWnd,WM_KEYUP,VK_LEFT,0);
		
		if(flag1==1)
		{	
			int x = device->getCursorControl()->getPosition().X;
			int y = device->getCursorControl()->getPosition().Y;
			if (x1>0)
			{
				x+=350;
			}
			else if (x1<0)
			{
				x-=350;
			}

			if (y1>0)
			{
				y+=350;
			}
			else if (y1<0)
			{
				y-=350;
			}
			device->getCursorControl()->setPosition(x,y);

			PostMessage(HWnd,WM_KEYUP,VK_DOWN,0);
			PostMessage(HWnd,WM_KEYUP,VK_UP,0);
			PostMessage(HWnd,WM_MOUSEMOVE,0,MAKELONG(x,y));
		}
		else
		{
			if(x1==-2)
			{
				PostMessage(HWnd,WM_KEYUP,VK_DOWN,0);
				PostMessage(HWnd,WM_KEYDOWN,VK_UP,0);
			}
			else if(x1==-1)
			{
				PostMessage(HWnd,WM_KEYUP,VK_UP,0);
				PostMessage(HWnd,WM_KEYDOWN,VK_DOWN,0);
			}
		}

		sendScreenshot();
		Sleep(10);
	}
}
