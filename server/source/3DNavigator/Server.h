#ifndef _SERVER_H_
#define _SERVER_H_

#include <list>
#include <vector>
#include <irrlicht.h>

#include "protocol.h"
#include "Resource.h"

class ServerUser;

class Server{
public:
	Server();
	~Server();

	void listenTo(unsigned short port = 52600);

	inline unsigned short getServerPort() const{return _server_port;}

	inline void setMaximumClientNum(int num){_maximum_client_num = num;}
	inline int getMaximumClientNum() const{return _maximum_client_num;}

	inline bool isListening() const{return _is_listening;}

	void disconnect(ServerUser *client);

	inline HANDLE getServerThread(){return _server_thread;}
	inline void setIndependentThreadEnabled(bool enabled){_is_independent_thread_enabled = enabled;}
	inline bool isIndependentThreadEnabled() const{return _is_independent_thread_enabled;}

	inline void setDriverType(irr::video::E_DRIVER_TYPE type){_driver_type = type;}
	inline irr::video::E_DRIVER_TYPE getDriverType() const{return _driver_type;}

	inline const std::list<ServerUser *> &getClients() const{return _clients;}
	
	inline const std::vector<Resource *> &getSceneMaps() const{return _scenemaps;};
	void loadSceneMap(const std::string &config_path);
	inline Resource *getSceneMapAt(int i){return _scenemaps.at(i);};

	void broadcastConfig(unsigned short port = 5261);

protected:
	R3D::TCPServer *_server_socket;
	R3D::UDPSocket *_broadcast_socket;

	bool _is_listening;
	unsigned short _server_port;
	unsigned short _broadcast_port;
	int _maximum_client_num;
	bool _is_independent_thread_enabled;
	irr::video::E_DRIVER_TYPE _driver_type;
	std::list<ServerUser *> _clients;
	std::vector<Resource *> _scenemaps;

private:
	static DWORD WINAPI _ServerThread(LPVOID lpParam);
	static DWORD WINAPI _BroadcastThread(LPVOID lpParam);

	HANDLE _server_thread;
	HANDLE _broadcast_thread;
};

extern Server *ServerInstance;

#endif
