#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <json/value.h>

namespace R3D{
	enum Command{
		Invalid,

		//Server to Client
		UpdateVideoFrame,
		

		//Client to Server
		SetResolution,

		Move,
		Scale,

		NumOfCommands
	};

	struct Packet{
		Command command;
		Json::Value args;

		inline Packet(Command command = Invalid){this->command = command;}
		std::string toString() const;
		static Packet FromString(const char *str);
	};
}

#endif
