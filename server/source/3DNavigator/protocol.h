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
		CreateDevice,

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
		Packet(const std::string &str);
		std::string toString() const;
	};

	enum Message{
		server_reaches_max_client_num,
		server_is_to_be_closed,
		loading_map
	};

	struct IP{
		unsigned char ip1, ip2, ip3, ip4;
		IP();
		IP(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4);
		IP(const sockaddr_in &ip);
		friend std::ostream &operator<<(std::ostream &cout, const IP &ip);
		operator unsigned() const;

		static const IP AnyHost;
		static const IP LocalHost;
		static const IP Broadcast;
	};

	class AbstractSocket{
	protected:
		SOCKET _socket;
		HANDLE _is_sending_data;

	public:
		AbstractSocket();
		AbstractSocket(SOCKET socket);
		~AbstractSocket();

		void bind(const IP &ip, unsigned short port);
		void connect(const IP &ip, unsigned short port);

		void write(const std::string &raw);
		void write(const char *data, int length);
		int read(char *buffer, int buffer_size);
		inline void close(){closesocket(_socket);}
		
	protected:
		virtual void _init() = 0;
	};

	class TCPSocket: public AbstractSocket{
	public:
		TCPSocket();
		TCPSocket(SOCKET socket);
		~TCPSocket();
		IP getPeerIp() const;

	protected:
		void _init() override;
	};

	class UDPSocket: public AbstractSocket{
	protected:
	public:
		UDPSocket();
		~UDPSocket();

		void setBroadcast(bool on);
		void receiveFrom(char *buffer, int size, IP &ip, unsigned short &port);
		void sendTo(const char *buffer, int size, const IP &ip, unsigned short port);

	protected:
		void _init() override;
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
		inline void setMaxClientNum(int num){_max_client_num = num;};
		bool listen(const IP &ip, unsigned short port);
		bool listen();
		TCPSocket *nextPendingConnection();
		inline void close(){closesocket(_socket);};
	};
}

#endif
