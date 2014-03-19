#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <json/value.h>

namespace R3D{
	enum Command{
		Invalid,

		//Server to Client
		UpdateVideoFrame,
		Quit,

		//Client to Server
		SetResolution,

		RotateCamera,
		ScaleCamera,
		MoveCamera,

		ControlHotspots,

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
