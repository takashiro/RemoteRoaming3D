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
	, _is_http_request(false)
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
	ServerUser *client = (ServerUser *) pParam;
	R3D::TCPSocket *&socket = client->_socket;

	const static int buffer_size = 96;
	const static int buffer_capacity = 128;
	char buffer[buffer_capacity];
	char *buffer_end = NULL;
	char *begin = NULL;
	char *end = NULL;
	int length = 0;

	while ((length = socket->receive(buffer, buffer_size)) > 0)
	{
		buffer_end = buffer + length;
		begin = buffer;
		while(true)
		{
			//find the end of the current line
			end = begin;
			while(*end != '\n' && end < buffer_end)
			{
				end++;
			}

			if(*end == '\n')
			{
				//we find a whole line, handle it
				*end = 0;
				client->handleCommand(begin);
				begin = end + 1;
				
				//if there's no more lines, go to receive data again
				if(begin >= buffer_end)
				{
					break;
				}
			}
			else
			{
				//only a left section is received, so we try to fill it
				buffer_end = buffer + buffer_capacity;
				while(end < buffer_end && socket->receive(end, 1) == 1 && *end != '\n')
				{
					end++;
				}

				if(*end == '\n')
				{
					*end = 0;
					client->handleCommand(begin);
				}

				break;
			}
		}
	}

	//disconnect the client
	ServerInstance->disconnect(client);

	return 0;
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

	if (_is_http_request) {
		sendPacket("HTTP/1.1 200 OK\n");
		sendPacket("Content-Type: image/jpeg\n");

		std::stringstream ss;
		ss << "Content-Length: " << length << "\n";
		sendPacket(ss.str());
		sendPacket("Connection : close\n\n");
		sendPacket(_memory_file->getContent(), length);
		return;
	} else {
		//transfer screenshot length
		R3D::Packet packet(R3D::UpdateVideoFrame);
		packet.args[0] = length;
		sendPacket(packet);
	}

	//transfer the picture
	sendPacket(_memory_file->getContent(), length);
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
	if (!_is_http_request) {
		R3D::Packet packet(R3D::MakeToastText);
		packet.args[0] = toastId;
		sendPacket(packet);
	}
}

void ServerUser::handleCommand(const char *cmd)
{
	if (_is_http_request) {
		if (*cmd == '\n') {
			_is_http_request = false;
		}
	}
	else {
		std::string url;
		if (strncmp(cmd, "GET ", 4) == 0){
			_is_http_request = true;
			std::stringstream ss;
			ss << cmd;
			std::string command;
			ss >> command >> url;
			url.erase(url.begin());
			urldecode(url);
			cmd = url.data();
		}

		R3D::Packet packet(cmd);
		std::map<R3D::Command, Callback>::iterator iter = _callbacks.find(packet.command);
		if (iter != _callbacks.end())
		{
			Callback func = iter->second;
			if (func != NULL){
				(this->*func)(packet.args);
				return;
			}
		}
		std::cout << "invalid packet (" << getIp() << "):" << cmd;
	}
}
