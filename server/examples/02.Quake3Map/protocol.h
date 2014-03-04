#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <json/value.h>

namespace R3D{
	struct Packet{
		enum Command{
			Invalid,

			//Server to Client
			UpdateVideoFrame,
		

			//Client to Server
			SetResolution,

			Move,
			Scale
		};
		Command command;
		
		Json::Value args;

		std::string toString() const;
		static Packet FromString(const char *str);
	};
}

#endif
