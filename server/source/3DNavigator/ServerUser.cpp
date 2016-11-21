#include "Server.h"
#include "ServerUser.h"
#include "Hotspot.h"

#include <memory.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace irr;

ServerUser::ServerUser(Server *server, R3D::TCPSocket *socket)
	: _server(server)
	, _socket(socket)
	, _device(NULL)
	, _device_thread(NULL)
	, _current_frame(NULL)
	, _receive_thread(NULL)
	, _memory_file(NULL)
	, _need_update(NULL)
	, _scene_map(NULL)
	, _hotspot_root(NULL)
	, _packet_handler(&ServerUser::handleConnection)
	, _read_socket(&ServerUser::readLine)
{
}

ServerUser::~ServerUser()
{
	disconnect();

	if(_need_update != NULL)
	{
		CloseHandle(_need_update);
	}

	if(_memory_file != NULL)
	{
		_memory_file->drop();
	}
}

DWORD WINAPI ServerUser::_ReceiveThread(LPVOID pParam){
	ServerUser *client = static_cast<ServerUser *>(pParam);
	R3D::TCPSocket *&socket = client->_socket;

	static int buffer_capacity = 8 * 1024;
	char *buffer = new char[buffer_capacity + 1];
	int length = 0;

	while ((length = socket->read(buffer, buffer_capacity)) > 0)
	{
		(client->*(client->_read_socket))(buffer, buffer_capacity, length);
	}

	//disconnect the client
	ServerInstance->disconnect(client);

	delete[] buffer;

	return 0;
}

void ServerUser::readLine(char *buffer, int buffer_capacity, int length)
{
	char *buffer_end = buffer + length;
	char *begin = buffer;
	char *end = nullptr;
	for (;;)
	{
		//find the end of the current line
		end = begin;
		while (*end != '\n' && *end != '\r' && end < buffer_end)
		{
			end++;
		}

		if (*end == '\n' || *end == '\r')
		{
			//we find a whole line, handle it
			if (*end == '\r') {
				*end = '\0';
				if (end + 1 < buffer_end) {
					if (end[1] == '\n') {
						end++;
						*end = '\0';
					}
				}
			}
			else {
				*end = '\0';
			}
			(this->*_packet_handler)(begin);
			begin = end + 1;

			//if there's no more lines, go to receive data again
			if (begin >= buffer_end)
			{
				break;
			}
		}
		else
		{
			//only a left section is received, break it
			break;
		}
	}
}

void ServerUser::readFrame(char *buffer, int buffer_size, int length)
{
	if (length <= 2) {
		return;
	}
	char *data = buffer + 2;
	length -= 2;

	unsigned long long payload_length = buffer[1] & 0x7F;
	if (payload_length == 126) {
		if (length < 4) {
			puts("Incomplete payload length");
			return;
		}
		payload_length = buffer[0] << 8 | buffer[1];
		data += 2;
		length -= 2;
	}
	else if (payload_length == 127) {
		if (length < 10) {
			puts("Incomplete payload length");
			return;
		}
		payload_length = 0;
		for (int i = 0; i < 8; i++) {
			payload_length <<= 8;
			payload_length |= data[i];
		}
		data += 8;
		length -= 8;
	}

	char masked = buffer[1] & 0x80;
	char *mask = nullptr;
	if (masked) {
		if (length < 4) {
			//Fatal Error
			puts("Incomplete masked key");
			return;
		}
		mask = data;
		data += 4;
		length -= 4;
	}

	if (length < payload_length) {
		puts("payload length exceeds array");
		return;
	}

	for (int i = 0, j = 0; i < payload_length; i++) {
		data[i] ^= mask[j];
		j++;
		if (j == 4) {
			j = 0;
		}
	}

	//Final Fragment
	//char type = buffer[0] & 0xF;
	if (buffer[0] & 0x80) {
		data[length] = '\0';
		handleCommand(data);
	}
	else {
		//Unsupported yet
	}
}

void ServerUser::disconnect()
{
	_socket->close();

	if(_device)
	{
		_device->closeDevice();
		ReleaseSemaphore(_need_update, 1, NULL);
	}
	
	if(_device_thread != NULL)
	{
		WaitForSingleObject(_device_thread, INFINITE);
		CloseHandle(_device_thread);
		_device_thread = NULL;
	}

	if(_receive_thread != NULL)
	{
		WaitForSingleObject(_receive_thread, INFINITE);
		CloseHandle(_receive_thread);
		_receive_thread = NULL;
	}
}

void ServerUser::startService()
{
	_receive_thread = CreateThread(NULL, 0, _ReceiveThread, (LPVOID) this, 0, NULL);
	_need_update = CreateSemaphore(NULL, 1, 1, NULL);
}

void ServerUser::createHotspots()
{
	if(!_hotspots.empty() || _scene_map == NULL)
		return;

	if(_scene_map->hotspot_path.empty())
		return;
	
	char buffer[3];
	std::ifstream hotspot_file(_scene_map->hotspot_path, std::ios::binary);
	hotspot_file.read(buffer, 3);

	//Handle UTF-8 BOM
	if(buffer[0] != '\xEF' || buffer[1] != '\xBB' || buffer[2] != '\xBF')
	{
		hotspot_file.putback(buffer[2]);
		hotspot_file.putback(buffer[1]);
		hotspot_file.putback(buffer[0]);
	}

	Json::Value hotspots;
	Json::Reader reader;
	bool success = reader.parse(hotspot_file, hotspots);
	hotspot_file.close();
	if(!success)
		return;

	scene::ISceneManager *smgr = _device->getSceneManager();
	_hotspot_root = smgr->addEmptySceneNode();

	std::wstring name;
	for(Json::Value::iterator i = hotspots.begin(); i != hotspots.end(); i++)
	{
		Hotspot *spot = new Hotspot(*i);
		copy_string(name, spot->getName());
		scene::IBillboardTextSceneNode *head_text = smgr->addBillboardTextSceneNode(0, name.c_str(), _hotspot_root, spot->getSize(), spot->getPosition());
		spot->setNode(head_text);
		_hotspots.push_back(spot);
	}
}

void ServerUser::clearHotspots()
{
	if(_hotspot_root == NULL || _hotspots.empty())
		return;

	static core::vector3df far_away(FLT_MIN, FLT_MIN, FLT_MIN);
	scene::ISceneManager *smgr = _device->getSceneManager();
	for(std::list<Hotspot *>::iterator i = _hotspots.begin(); i != _hotspots.end(); i++)
	{
		Hotspot *&spot = *i;
		scene::ISceneNode *node = spot->getNode();
		node->setPosition(far_away);
		delete spot;
	}

	_hotspots.clear();
	smgr->addToDeletionQueue(_hotspot_root);
	_hotspot_root = NULL;
}

void ServerUser::sendScreenshot()
{
	if(_device == NULL || _current_frame == NULL)
	{
		return;
	}

	_memory_file->clear();
	if(!_device->getVideoDriver()->writeImageToFile(_current_frame, _memory_file, 50))
	{
		puts("failed to transfer video frame");
	}

	int length = (int) _memory_file->getPos();

	std::string raw(_memory_file->getContent(), length);
	R3D::Packet packet(R3D::UpdateVideoFrame);
	packet.args = base64_encode(raw);
	sendPacket(packet);
}

void ServerUser::enterHotspot(Hotspot *spot)
{
	R3D::Packet packet(R3D::EnterHotspot);
	packet.args = spot->toJson();
	sendPacket(packet);
}

void ServerUser::getIp(std::wstring &wstr)
{
	std::wstringstream buffer;
	buffer << getIp();
	buffer >> wstr;
}

void ServerUser::makeToast(int toastId)
{
	R3D::Packet packet(R3D::MakeToastText);
	packet.args[0] = toastId;
	sendPacket(packet);
}

void ServerUser::handleConnection(const char *cmd)
{
	if (*cmd == '[') {
		_packet_handler = &ServerUser::handleCommand;
		handleCommand(cmd);
	}
	else {
		if (strnicmp(cmd, "GET ", 4) != 0) {
			_packet_handler = &ServerUser::handleWebSocket;
		}
	}
}

void ServerUser::handleWebSocket(const char *cmd)
{
	if (strnicmp(cmd, "Sec-WebSocket-Key: ", 19) == 0) {
		std::string sec_key = cmd + 19;
		sec_key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

		std::string key_accept = base64_encode(sha1(sec_key));

		_socket->write("HTTP/1.1 101 Switching Protocols\n");
		_socket->write("Upgrade: websocket\n");
		_socket->write("Connection: Upgrade\n");
		_socket->write("Sec-WebSocket-Accept: ");
		_socket->write(key_accept);
		_socket->write("\n\n");
	}
	else if (*cmd == '\0') {
		_packet_handler = &ServerUser::handleCommand;
		_read_socket = &ServerUser::readFrame;
	}
}

void ServerUser::handleCommand(const char *cmd)
{
	if (*cmd != '[') {
		return;
	}

	R3D::Packet packet(cmd);
	std::map<R3D::Command, Callback>::iterator iter = _callbacks.find(packet.command);
	if (iter != _callbacks.end())
	{
		Callback func = iter->second;
		if (func) {
			(this->*func)(packet.args);
			return;
		}
	}
	std::cout << "invalid packet (" << getIp() << "):" << cmd;
}
