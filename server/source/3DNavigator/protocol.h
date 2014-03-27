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
		operator unsigned();

		static const IP AnyHost;
	};

	class TCPSocket{
	protected:
		SOCKET _socket;
		HANDLE _is_sending_data;

	public:
		TCPSocket(SOCKET socket);
		~TCPSocket();
		
		void send(const std::string &raw);
		inline void send(const R3D::Packet &packet){send(packet.toString());};
		bool receive(char *buffer, int buffer_size);
		IP getPeerIp() const;

		inline void close(){::closesocket(_socket);};
	};

	class UDPSocket{
	};

	class TCPServer{
	protected:
		IP _ip;
		unsigned short _port;
		SOCKET _socket;
		int _max_client_num;

	public:
		TCPServer();
		~TCPServer();
		TCPServer(const IP &ip, unsigned short port);
		inline void setMaxClientNum(int num){_max_client_num = num;};
		bool listen(const IP &ip, unsigned short port);
		bool listen();
		TCPSocket *nextPendingConnection();
		inline void close(){closesocket(_socket);};
	};
}

#endif
