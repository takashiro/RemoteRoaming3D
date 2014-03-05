#ifndef _SERVERUSER_H_
#define _SERVERUSER_H_

#include <WinSock.h>
#include <irrlicht.h>

class Server;

class ServerUser{
public:
	ServerUser(Server *server, SOCKET socket);
	~ServerUser();

	inline bool isValid() const{return _receive_thread != NULL;}
	inline irr::IrrlichtDevice *getDevice(){return _device;}

protected:
	void sendScreenshot();
	void handleCommand(const char *cmd);

	Server *_server;
	SOCKET _socket;
	irr::IrrlichtDevice *_device;

private:
	static DWORD WINAPI _ReceiveThread(LPVOID lpParam);
	HANDLE _receive_thread;

	static DWORD WINAPI _DeviceThread(LPVOID lpParam);
	HANDLE _device_thread;
};

#endif
