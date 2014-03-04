#ifndef _SERVER_H_
#define _SERVER_H_

#include <list>

#include <WinSock.h>
#pragma comment(lib,"ws2_32.lib")

#include <IrrlichtDevice.h>

class Client;

class Server{
public:
	Server(irr::IrrlichtDevice *device);
	~Server();

	void listenTo(short port = 6666);
	void pendingNewConnection();

	inline void setServerPort(short port){_server_port = port;}
	inline short getServerPort() const{return _server_port;}

	inline void setMaximumClientNum(int num){_maximum_client_num = num;}
	inline int getMaximumClientNum() const{return _maximum_client_num;}

	inline irr::IrrlichtDevice *getIrrlichtDevice(){return _device;}

	inline bool isListening() const{return _is_listening;}

	void disconnect(Client *client);

protected:
	SOCKET _server_socket;
	irr::IrrlichtDevice *_device;

	bool _is_listening;
	short _server_port;
	int _maximum_client_num;

private:
	static DWORD WINAPI _ServerThread(LPVOID lpParam);

	HANDLE _server_thread;
	std::list<Client *> _clients;
};

#endif
