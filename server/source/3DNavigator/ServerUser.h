#pragma once

#include "global.h"

#include <irrlicht.h>
#include <json/json.h>
#include <map>
#include <list>

#include "protocol.h"
#include "IrrMemoryFile.h"

RD_NAMESPACE_BEGIN

class Server;
class Hotspot;
class Thread;
class Semaphore;
struct SceneMap;

class ServerUser
{
public:
	ServerUser(Server *server, R3D::TCPSocket *socket);
	~ServerUser();

	inline bool isValid() const { return mReceiveThread != NULL; }
	inline irr::IrrlichtDevice *getDevice() { return mDevice; }
	inline R3D::IP getIp() const { return mSocket->getPeerIp(); };
	void getIp(std::wstring &str);

	void sendPacket(const R3D::Packet &packet);
	void disconnect();

	void startService();
	void makeToast(int toastId);

protected:
	void sendScreenshot();
	void enterHotspot(Hotspot *spot);
	void createHotspots();
	void clearHotspots();

	void readLine(char *buffer, int buffer_capacity, int length);
	void readFrame(char *buffer, int buffer_capacity, int length);

	void handleConnection(const char *cmd);
	void handleWebSocket(const char *cmd);
	void handleCommand(const char *cmd);

	typedef void (ServerUser::*Callback)(const Json::Value &args);
	static std::map<R3D::Command, Callback> mCallbacks;

	void createDeviceCommand(const Json::Value &args);
	void rotateCameraCommand(const Json::Value &args);
	void scaleCameraCommand(const Json::Value &args);
	void moveCameraCommand(const Json::Value &args);
	void controlHotspotsCommand(const Json::Value &args);
	void doubleClickCommand(const Json::Value &args);

	int mScreenWidth;
	int mScreenHeight;

	Server *mServer;
	R3D::TCPSocket *mSocket;
	irr::IrrlichtDevice *mDevice;
	irr::video::IImage *mCurrentFrame;
	std::list<Hotspot *> mHotspots;
	irr::io::MemoryFile *mMemoryFile;
	SceneMap *mSceneMap;
	irr::scene::ISceneNode *mHotspotRoot;
	void (ServerUser::*mPacketHandler)(const char *);
	void (ServerUser::*mReadSocket)(char *buffer, int buffer_capacity, int length);

private:
	Thread *mReceiveThread;
	Thread *mDeviceThread;
	Semaphore *mNeedUpdate;

public:
	class CallbackAdder
	{
	public:
		CallbackAdder();
	};
};

RD_NAMESPACE_END
