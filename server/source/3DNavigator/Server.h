#pragma once

#include <list>
#include <vector>
#include <irrlicht.h>

#include "protocol.h"
#include "SceneMap.h"

RD_NAMESPACE_BEGIN

class ServerUser;

class Server
{
public:
	Server();
	~Server();

	void listenTo(ushort port = 52600);

	inline ushort getServerPort() const { return mServerPort; }

	inline void setMaximumClientNum(int num) { mMaximumClientNum = num; }
	inline int getMaximumClientNum() const { return mMaximumClientNum; }

	inline bool isListening() const { return mIsListening; }

	void disconnect(ServerUser *client);

	inline HANDLE getServerThread() { return mServerThread; }
	inline void setIndependentThreadEnabled(bool enabled) { mIsIndependentThreadEnabled = enabled; }
	inline bool isIndependentThreadEnabled() const { return mIsIndependentThreadEnabled; }

	inline void setDriverType(irr::video::E_DRIVER_TYPE type) { mDriverType = type; }
	inline irr::video::E_DRIVER_TYPE getDriverType() const { return mDriverType; }

	inline const std::list<ServerUser *> &getClients() const { return mClients; }

	inline const std::vector<SceneMap *> &getSceneMaps() const { return mScenemaps; };
	void loadSceneMap(const std::string &config_path);
	inline SceneMap *getSceneMapAt(int i) { return mScenemaps.at(i); };

	void broadcastConfig(ushort port = 5261);

protected:
	R3D::TCPServer *mServerSocket;
	R3D::UDPSocket *mBroadcastSocket;

	bool mIsListening;
	bool mIsBroadcastingConfig;
	ushort mServerPort;
	ushort mBroadcastPort;
	int mMaximumClientNum;
	bool mIsIndependentThreadEnabled;
	irr::video::E_DRIVER_TYPE mDriverType;
	std::list<ServerUser *> mClients;
	std::vector<SceneMap *> mScenemaps;

private:
	static DWORD WINAPI ServerThread(LPVOID lpParam);
	static DWORD WINAPI BroadcastThread(LPVOID lpParam);

	HANDLE mServerThread;
	HANDLE mBroadcastThread;
};

extern Server *ServerInstance;

RD_NAMESPACE_END
