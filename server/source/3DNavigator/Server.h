#ifndef _SERVER_H_
#define _SERVER_H_

#include <list>
#include <irrlicht.h>
#include <WinSock.h>

class ServerUser;

class Server{
public:
	Server();
	~Server();

	void listenTo(short port = 6666);

	inline void setServerPort(short port){_server_port = port;}
	inline short getServerPort() const{return _server_port;}

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

protected:
	SOCKET _server_socket;

	bool _is_listening;
	short _server_port;
	int _maximum_client_num;
	bool _is_independent_thread_enabled;
	irr::video::E_DRIVER_TYPE _driver_type;

private:
	static DWORD WINAPI _ServerThread(LPVOID lpParam);

	HANDLE _server_thread;
	std::list<ServerUser *> _clients;
};

extern Server *ServerInstance;

#endif
