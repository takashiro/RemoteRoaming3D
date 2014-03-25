#include "protocol.h"
#include <json/json.h>

namespace R3D{

const IP IP::AnyHost(0, 0, 0, 0);

std::string Packet::toString() const{
	Json::Value packet;
	packet[0] = (int) command;
	packet[1] = args;
	
	Json::FastWriter writer;
	return writer.write(packet);
}

Packet Packet::FromString(const char *str){
	Packet packet;
	
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(str, value)){
		packet.args = Json::nullValue;
	}else{
		packet.command = (Command) value[0].asInt();
		packet.args = value[1];
	}
	
	return packet;
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

SocketAddress::SocketAddress(const sockaddr_in &addr)
	:ip(addr)
{
	port = addr.sin_port;
}

SocketAddress::SocketAddress(const IP &ip, unsigned short port)
	:ip(ip), port(port)
{
}

std::ostream &operator<<(std::ostream &cout, const IP &ip)
{
	cout << (short) ip.ip1 << "."
		<< (short) ip.ip2 << "."
		<< (short) ip.ip3 << "."
		<< (short) ip.ip4;
	return cout;
};

std::ostream &operator<<(std::ostream &cout, const SocketAddress &address)
{
	cout << address.ip << ":" << address.port;
	return cout;
};

}
