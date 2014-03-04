#ifndef _SERVERUSER_H_
#define _SERVERUSER_H_

#include <WinSock.h>

class Server;

class ServerUser{
public:
	ServerUser(Server *server, SOCKET socket);
	~ServerUser();

	inline bool isValid() const{return _receive_thread != NULL;}

protected:
	void sendScreenshot();
	void handleCommand(const char *cmd);

	Server *_server;
	SOCKET _socket;
	

private:
	static DWORD WINAPI _ReceiveThread(LPVOID lpParam);
	HANDLE _receive_thread;
};

#endif
