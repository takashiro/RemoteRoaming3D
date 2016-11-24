#include "protocol.h"
#include "Semaphore.h"
#include <json/json.h>

RD_NAMESPACE_BEGIN

const IP IP::AnyHost(0, 0, 0, 0);
const IP IP::LocalHost(127, 0, 0, 1);
const IP IP::Broadcast(255, 255, 255, 255);

Packet::Packet(const std::string &str)
{
	Json::Reader reader;
	Json::Value value;
	if (!reader.parse(str, value)) {
		args = Json::nullValue;
	} else {
		command = (Command)value[0].asInt();
		args = value[1];
	}
}

std::string Packet::toString() const
{
	Json::Value packet;
	packet[0] = static_cast<int>(command);
	packet[1] = args;

	Json::FastWriter writer;
	return writer.write(packet);
}

IP::IP()
{
	ip1 = ip2 = ip3 = ip4 = 0;
}

IP::IP(uchar ip1, uchar ip2, uchar ip3, uchar ip4)
	:ip1(ip1), ip2(ip2), ip3(ip3), ip4(ip4)
{
}

IP::IP(const sockaddr_in &ip)
{
	ip1 = ip.sin_addr.S_un.S_un_b.s_b1;
	ip2 = ip.sin_addr.S_un.S_un_b.s_b2;
	ip3 = ip.sin_addr.S_un.S_un_b.s_b3;
	ip4 = ip.sin_addr.S_un.S_un_b.s_b4;
}

IP::operator uint() const
{
	uint ip = ip1;
	ip <<= 8;
	ip |= ip2;
	ip <<= 8;
	ip |= ip3;
	ip <<= 8;
	ip |= ip4;
	return ip;
}

std::ostream &operator<<(std::ostream &cout, const IP &ip)
{
	cout << (short)ip.ip1 << "."
		<< (short)ip.ip2 << "."
		<< (short)ip.ip3 << "."
		<< (short)ip.ip4;
	return cout;
};

TCPServer::TCPServer()
{
	mPort = 0;
	mMaxClientNum = 0;
}

TCPServer::~TCPServer()
{
	close();
}

bool TCPServer::listen(const IP &ip, ushort port)
{
	mIp = ip;
	mPort = port;
	return listen();
}

bool TCPServer::listen()
{
	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData)) {
		return false;
	}

	//try to create a TCP socket
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == mSocket) {
		WSACleanup();
		return false;
	}

	//listen to any ip address of the local host
	sockaddr_in addr_sev;
	addr_sev.sin_family = AF_INET;
	addr_sev.sin_port = htons(mPort);
	addr_sev.sin_addr.s_addr = static_cast<uint>(mIp);
	if (SOCKET_ERROR == bind(mSocket, (sockaddr *)&addr_sev, sizeof(addr_sev))) {
		WSACleanup();
		return false;
	}
	if (SOCKET_ERROR == ::listen(mSocket, mMaxClientNum)) {
		WSACleanup();
		return false;
	}

	return true;
}

TCPSocket *TCPServer::nextPendingConnection()
{
	sockaddr_in client_addr;
	int nAddrLen = sizeof(client_addr);

	SOCKET client_socket = accept(mSocket, (sockaddr *)&client_addr, &nAddrLen);
	if (client_socket == INVALID_SOCKET) {
		return NULL;
	}

	return new TCPSocket(client_socket);
}

AbstractSocket::AbstractSocket()
	:mSocket(NULL)
{
	mIsSendingData = new Semaphore;
}

AbstractSocket::AbstractSocket(SOCKET socket)
	: mSocket(socket)
{
	mIsSendingData = new Semaphore;
}

AbstractSocket::~AbstractSocket()
{
	delete mIsSendingData;
	close();
}

void AbstractSocket::bind(const IP &ip, ushort port)
{
	//bind the IP and the port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = (unsigned int)ip;
	if (SOCKET_ERROR == ::bind(mSocket, (sockaddr *)&addr, sizeof(addr))) {
		WSACleanup();
	}
}

void AbstractSocket::connect(const IP &ip, ushort port)
{
	//connect the IP and the port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = (uint)ip;
	if (SOCKET_ERROR == ::connect(mSocket, (sockaddr *)&addr, sizeof(addr))) {
		WSACleanup();
	}
}

void AbstractSocket::write(const std::string &raw)
{
	mIsSendingData->acquire();

	size_t length = raw.length();
	size_t y = 0;
	const char *p = raw.c_str();
	while (y < length) {
		y += ::send(mSocket, p + y, int(length - y), 0);
	}

	mIsSendingData->release();
}

void AbstractSocket::write(const char *raw, int length)
{
	mIsSendingData->acquire();

	int y = 0;
	while (y < length) {
		int result = ::send(mSocket, raw + y, int(length - y), 0);
		if (result == SOCKET_ERROR) {
			break;
		} else {
			y += result;
		}
	}

	mIsSendingData->release();
}

int AbstractSocket::read(char *buffer, int buffer_size)
{
	return recv(mSocket, buffer, buffer_size, 0);
}

TCPSocket::TCPSocket()
{
	init();
}

TCPSocket::TCPSocket(SOCKET socket)
	:AbstractSocket(socket)
{
}

TCPSocket::~TCPSocket()
{
}

IP TCPSocket::getPeerIp() const
{
	sockaddr_in ip;
	int length = sizeof(ip);
	getpeername(mSocket, (sockaddr *)&ip, &length);
	return ip;
}

void TCPSocket::init()
{
	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData)) {
		return;
	}

	//try to create a UDP socket
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == mSocket) {
		WSACleanup();
		return;
	}
}

UDPSocket::UDPSocket()
{
	init();
}

UDPSocket::~UDPSocket()
{
}

void UDPSocket::receiveFrom(char *buffer, int size, IP &ip, ushort &port)
{
	sockaddr_in addr;
	int addrlen = sizeof(addr);
	recvfrom(mSocket, buffer, size, 0, (sockaddr *)&addr, &addrlen);
	ip = addr;
	port = addr.sin_port;
}

void UDPSocket::sendTo(const char *buffer, int size, const IP &ip, ushort port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_un_b.s_b1 = ip.ip1;
	addr.sin_addr.S_un.S_un_b.s_b2 = ip.ip2;
	addr.sin_addr.S_un.S_un_b.s_b3 = ip.ip3;
	addr.sin_addr.S_un.S_un_b.s_b4 = ip.ip4;

	mIsSendingData->acquire();

	int y = 0;
	while (y < size) {
		int result = sendto(mSocket, buffer, size, 0, (const sockaddr *)&addr, sizeof(addr));
		if (result == SOCKET_ERROR) {
			break;
		} else {
			y += result;
		}
	}

	mIsSendingData->release();
}

void UDPSocket::setBroadcast(bool on)
{
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, (char FAR *)&on, sizeof(on))) {
		puts("setsockopt failed");
		return;
	}
}

void UDPSocket::init()
{
	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData)) {
		return;
	}

	//try to create a UDP socket
	mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == mSocket) {
		WSACleanup();
		return;
	}
}

RD_NAMESPACE_END
