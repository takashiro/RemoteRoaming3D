#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <json/value.h>
#include <winsock.h>
#include <ostream>

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

	struct IP{
		unsigned char ip1, ip2, ip3, ip4;
		IP();
		IP(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4);
		IP(const sockaddr_in &ip);
		friend std::ostream &operator<<(std::ostream &cout, const IP &ip);

		static const IP AnyHost;
	};

	struct SocketAddress{
		IP ip;
		unsigned short port;

		SocketAddress(const sockaddr_in &ip);
		SocketAddress(const IP &ip, unsigned short port);
		friend std::ostream &operator<<(std::ostream &cout, const SocketAddress &ip);
	};
}

#endif
