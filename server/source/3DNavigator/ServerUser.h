#ifndef _SERVERUSER_H_
#define _SERVERUSER_H_

#include <irrlicht.h>
#include <json/json.h>
#include <map>
#include <list>

#include "protocol.h"
#include "util.h"
#include "IrrMemoryFile.h"

class Server;
class Hotspot;
struct SceneMap;

class ServerUser{
public:
	ServerUser(Server *server, R3D::TCPSocket *socket);
	~ServerUser();

	inline bool isValid() const{return _receive_thread != NULL;}
	inline irr::IrrlichtDevice *getDevice(){return _device;}
	inline R3D::IP getIp() const{return _socket->getPeerIp();};
	void getIp(std::wstring &str);

	inline void sendPacket(const R3D::Packet &packet){ _socket->send(packet.toString()); };
	inline void sendPacket(const std::string &raw){_socket->send(raw);};
	inline void sendPacket(const char *raw, int length){_socket->send(raw, length);};
	void disconnect();

	void startService();
	void makeToast(int toastId);

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
	R3D::TCPSocket *_socket;
	irr::IrrlichtDevice *_device;
	irr::video::IImage *_current_frame;
	std::list<Hotspot *> _hotspots;
	IrrMemoryFile *_memory_file;
	SceneMap *_scene_map;
	irr::scene::ISceneNode *_hotspot_root;
	bool _is_http_request;

private:
	static DWORD WINAPI _ReceiveThread(LPVOID lpParam);
	HANDLE _receive_thread;

	static DWORD WINAPI _DeviceThread(LPVOID lpParam);
	HANDLE _device_thread;
	HANDLE _need_update;

public:
	class CallbackAdder{
	public:
		CallbackAdder();
	};
};

#endif
