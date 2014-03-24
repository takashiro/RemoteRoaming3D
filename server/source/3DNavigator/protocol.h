#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <json/value.h>

namespace R3D{
	enum Command{
		Invalid,

		//Server to Client
		UpdateVideoFrame,
		MakeToastText,
		EnterHotspot,
		Quit,

		//Client to Server
		SetResolution,

		RotateCamera,
		ScaleCamera,
		MoveCamera,

		ControlHotspots,
		DoubleClick,

		NumOfCommands
	};

	struct Packet{
		Command command;
		Json::Value args;

		inline Packet(Command command = Invalid){this->command = command;}
		std::string toString() const;
		static Packet FromString(const char *str);
	};

	enum Message{
		server_reaches_max_client_num,
		server_is_to_be_closed
	};
}

#endif
