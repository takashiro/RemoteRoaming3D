#include "protocol.h"
#include <json/reader.h>

using namespace R3D;

std::string Packet::toString() const{
	Json::Value packet;
	packet[0] = (int) command;
	packet[1] = args;
	return packet.toStyledString();
}

Packet Packet::FromString(const char *str){
	Packet packet;
	
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(str, value)){
		packet.command = Packet::Invalid;
		packet.args = Json::nullValue;
	}else{
		packet.command = (Packet::Command) value[0].asInt();
		packet.args = value[1];
	}
	
	return packet;
}
