#ifndef _SERVERUSER_H_
#define _SERVERUSER_H_

#include <WinSock.h>
#include <irrlicht.h>
#include <json/json.h>
#include <map>

#include "protocol.h"

class Server;

class ServerUser{
public:
	ServerUser(Server *server, SOCKET socket);
	~ServerUser();

	inline bool isValid() const{return _receive_thread != NULL;}
	inline irr::IrrlichtDevice *getDevice(){return _device;}
	sockaddr_in getIp();

	void disconnect();

protected:
	void sendScreenshot();
	void handleCommand(const char *cmd);

	typedef void (ServerUser::*Callback)(const Json::Value &args);
	std::map<R3D::Command, Callback> _callbacks;

	void _createDevice(const Json::Value &args);
	void _moveCamera(const Json::Value &args);
	void _scaleCamera(const Json::Value &args);

	int _screen_width;
	int _screen_height;

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
