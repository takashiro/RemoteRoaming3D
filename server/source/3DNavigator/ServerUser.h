#ifndef _SERVERUSER_H_
#define _SERVERUSER_H_

#include <WinSock.h>
#include <irrlicht.h>
#include <json/json.h>
#include <map>
#include <list>

#include "protocol.h"
#include "util.h"

class Server;
class Hotspot;

class ServerUser{
public:
	ServerUser(Server *server, SOCKET socket);
	~ServerUser();

	inline bool isValid() const{return _receive_thread != NULL;}
	inline irr::IrrlichtDevice *getDevice(){return _device;}
	R3D::IP getIp() const;
	void getIp(std::wstring &str);

	inline void sendPacket(const R3D::Packet &packet){sendPacket(packet.toString());};
	void sendPacket(const std::string &raw);
	void disconnect();

	void startService();

protected:
	void sendScreenshot();
	void enterHotspot(Hotspot *spot);
	void handleCommand(const char *cmd);
	void createHotspots();
	void clearHotspots();

	typedef void (ServerUser::*Callback)(const Json::Value &args);
	static std::map<R3D::Command, Callback> _callbacks;

	void _createDevice(const Json::Value &args);
	void _rotateCamera(const Json::Value &args);
	void _scaleCamera(const Json::Value &args);
	void _moveCamera(const Json::Value &args);
	void _controlHotspots(const Json::Value &args);
	void _doubleClick(const Json::Value &args);

	int _screen_width;
	int _screen_height;

	Server *_server;
	SOCKET _socket;
	irr::IrrlichtDevice *_device;
	irr::video::IImage *_current_frame;
	std::list<Hotspot *> _hotspots;

private:
	static DWORD WINAPI _ReceiveThread(LPVOID lpParam);
	HANDLE _receive_thread;

	static DWORD WINAPI _DeviceThread(LPVOID lpParam);
	HANDLE _device_thread;
	HANDLE _need_update;
	HANDLE _is_sending_data;
};

#endif
