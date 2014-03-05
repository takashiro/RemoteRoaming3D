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
		Scale
	};

	struct Packet{
		Command command;
		
		Json::Value args;

		std::string toString() const;
		static Packet FromString(const char *str);
	};
}

#endif
