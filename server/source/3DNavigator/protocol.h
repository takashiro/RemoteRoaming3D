#pragma once

#include "global.h"

#include <json/value.h>
#include <winsock.h>
#include <ostream>

RD_NAMESPACE_BEGIN

class Semaphore;

enum Command
{
	Invalid,

	//Server to Client
	UpdateVideoFrame,
	MakeToastText,
	EnterHotspot,
	Quit,

	//Client to Server
	CreateDevice,

	RotateCamera,
	ScaleCamera,
	MoveCamera,

	ControlHotspots,
	DoubleClick,

	NumOfCommands
};

struct Packet
{
	Command command;
	Json::Value args;

	inline Packet(Command command = Invalid) { this->command = command; }
	Packet(const std::string &str);
	std::string toString() const;
};

enum Message
{
	server_reaches_max_client_num,
	server_is_to_be_closed,
	loading_map
};

struct IP
{
	uchar ip1, ip2, ip3, ip4;
	IP();
	IP(uchar ip1, uchar ip2, uchar ip3, uchar ip4);
	IP(const sockaddr_in &ip);
	friend std::ostream &operator<<(std::ostream &cout, const IP &ip);
	operator uint() const;

	static const IP AnyHost;
	static const IP LocalHost;
	static const IP Broadcast;
};

class AbstractSocket
{
protected:
	SOCKET mSocket;
	Semaphore *mIsSendingData;

public:
	AbstractSocket();
	AbstractSocket(SOCKET socket);
	~AbstractSocket();

	void bind(const IP &ip, ushort port);
	void connect(const IP &ip, ushort port);

	void write(const std::string &raw);
	void write(const char *data, int length);
	int read(char *buffer, int buffer_size);
	inline void close() { closesocket(mSocket); }

protected:
	virtual void init() = 0;
};

class TCPSocket : public AbstractSocket
{
public:
	TCPSocket();
	TCPSocket(SOCKET socket);
	~TCPSocket();
	IP getPeerIp() const;

protected:
	void init() override;
};

class UDPSocket : public AbstractSocket
{
protected:
public:
	UDPSocket();
	~UDPSocket();

	void setBroadcast(bool on);
	void receiveFrom(char *buffer, int size, IP &ip, ushort &port);
	void sendTo(const char *buffer, int size, const IP &ip, ushort port);

protected:
	void init() override;
};

class TCPServer
{
protected:
	IP mIp;
	ushort mPort;
	SOCKET mSocket;
	int mMaxClientNum;

public:
	TCPServer();
	~TCPServer();
	inline void setMaxClientNum(int num) { mMaxClientNum = num; };
	bool listen(const IP &ip, ushort port);
	bool listen();
	TCPSocket *nextPendingConnection();
	inline void close() { closesocket(mSocket); };
};

RD_NAMESPACE_END
