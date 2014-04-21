#include "protocol.h"
#include <json/json.h>

namespace R3D{

const IP IP::AnyHost(0, 0, 0, 0);
const IP IP::LocalHost(127, 0, 0, 1);
const IP IP::Broadcast(255, 255, 255, 255);


Packet::Packet(const std::string &str)
{
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(str, value)){
		args = Json::nullValue;
	}else{
		command = (Command) value[0].asInt();
		args = value[1];
	}
}

std::string Packet::toString() const{
	Json::Value packet;
	packet[0] = (int) command;
	packet[1] = args;
	
	Json::FastWriter writer;
	return writer.write(packet);
}

IP::IP()
{
	ip1 = ip2 = ip3 = ip4 = 0;
}

IP::IP(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4)
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

IP::operator unsigned() const
{
	unsigned ip = ip1;
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
	cout << (short) ip.ip1 << "."
		<< (short) ip.ip2 << "."
		<< (short) ip.ip3 << "."
		<< (short) ip.ip4;
	return cout;
};

TCPServer::TCPServer()
{
	_port = 0;
	_max_client_num = 0;
}

TCPServer::~TCPServer()
{
	close();
}

bool TCPServer::listen(const IP &ip, unsigned short port)
{
	_ip = ip;
	_port = port;
	return listen();
}

bool TCPServer::listen()
{
	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData))
	{
		return false;
	}
	
	//try to create a TCP socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _socket)
	{
		WSACleanup();
		return false;
	}
	
	//listen to any ip address of the local host
	sockaddr_in addr_sev;
	addr_sev.sin_family = AF_INET;
	addr_sev.sin_port = htons(_port);
	addr_sev.sin_addr.s_addr = (unsigned int) _ip;
	if (SOCKET_ERROR == bind(_socket, (sockaddr *)&addr_sev, sizeof(addr_sev)))
	{
		WSACleanup();
		return false;
	}
	if (SOCKET_ERROR == ::listen(_socket, _max_client_num))
	{
		WSACleanup();
		return false;
	}

	return true;
}

TCPSocket *TCPServer::nextPendingConnection()
{
	sockaddr_in client_addr;
	int nAddrLen = sizeof(client_addr);

	SOCKET client_socket = accept(_socket, (sockaddr *)&client_addr, &nAddrLen);
	if (client_socket == INVALID_SOCKET)
	{
		return NULL;
	}

	return new TCPSocket(client_socket);
}

AbstractSocket::AbstractSocket()
	:_socket(NULL)
{
	_is_sending_data = CreateSemaphore(NULL, 1, 1, NULL);
}

AbstractSocket::AbstractSocket(SOCKET socket)
	:_socket(socket)
{
	_is_sending_data = CreateSemaphore(NULL, 1, 1, NULL);
}

AbstractSocket::~AbstractSocket()
{
	CloseHandle(_is_sending_data);
	close();
}

void AbstractSocket::bind(const IP &ip, unsigned short port)
{
	//bind the IP and the port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = (unsigned int) ip;
	if (SOCKET_ERROR == ::bind(_socket, (sockaddr *)&addr, sizeof(addr)))
	{
		WSACleanup();
	}
}

void AbstractSocket::connect(const IP &ip, unsigned short port)
{	
	//connect the IP and the port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = (unsigned int) ip;
	if (SOCKET_ERROR == ::connect(_socket, (sockaddr *)&addr, sizeof(addr)))
	{
		WSACleanup();
	}
}

void AbstractSocket::send(const std::string &raw)
{
	WaitForSingleObject(_is_sending_data, INFINITE);

	size_t length = raw.length();
	size_t y = 0;
	const char *p = raw.c_str();
	while(y < length)
	{
		y += ::send(_socket, p + y, int(length - y), 0);
	}

	ReleaseSemaphore(_is_sending_data, 1, NULL);
}

void AbstractSocket::send(const char *raw, int length)
{
	WaitForSingleObject(_is_sending_data, INFINITE);

	int y = 0;
	while(y < length)
	{
		int result = ::send(_socket, raw + y, int(length - y), 0);
		if(result == SOCKET_ERROR)
		{
			break;
		}
		else
		{
			y += result;
		}
	}

	ReleaseSemaphore(_is_sending_data, 1, NULL);
}

int AbstractSocket::receive(char *buffer, int buffer_size)
{
	return recv(_socket, buffer, buffer_size, 0);
}

TCPSocket::TCPSocket()
{
	_init();
}

TCPSocket::TCPSocket(SOCKET socket)
	:AbstractSocket(socket)
{
}

TCPSocket::~TCPSocket()
{
}

IP TCPSocket::getPeerIp() const{
	sockaddr_in ip;
	int length = sizeof(ip);
	getpeername(_socket, (sockaddr *) &ip, &length);
	return ip;
}

void TCPSocket::_init()
{
	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData))
	{
		return;
	}
	
	//try to create a UDP socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _socket)
	{
		WSACleanup();
		return;
	}
}

UDPSocket::UDPSocket()
{
	_init();
}

UDPSocket::~UDPSocket()
{
}

void UDPSocket::receiveFrom(char *buffer, int size, IP &ip, unsigned short &port)
{
	sockaddr_in addr;
	int addrlen = sizeof(addr);
	recvfrom(_socket, buffer, size, 0, (sockaddr *) &addr, &addrlen);
	ip = addr;
	port = addr.sin_port;
}

void UDPSocket::sendTo(const char *buffer, int size, const IP &ip, unsigned short port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_un_b.s_b1 = ip.ip1;
	addr.sin_addr.S_un.S_un_b.s_b2 = ip.ip2;
	addr.sin_addr.S_un.S_un_b.s_b3 = ip.ip3;
	addr.sin_addr.S_un.S_un_b.s_b4 = ip.ip4;

	WaitForSingleObject(_is_sending_data, INFINITE);

	int y = 0;
	while(y < size)
	{
		int result = sendto(_socket, buffer, size, 0, (const sockaddr *) &addr, sizeof(addr));
		if(result == SOCKET_ERROR)
		{
			break;
		}
		else
		{
			y += result;
		}
	}

	ReleaseSemaphore(_is_sending_data, 1, NULL);
}

void UDPSocket::setBroadcast(bool on)
{
	if(SOCKET_ERROR == setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char FAR *)&on, sizeof(on)))
	{
		puts("setsockopt failed");
		return;
	}
}

void UDPSocket::_init()
{
	//start up windows socket service of the corresponding version 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	if (WSAStartup(sockVersion, &wsaData))
	{
		return;
	}
	
	//try to create a UDP socket
	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == _socket)
	{
		WSACleanup();
		return;
	}
}

}
