#include "Server.h"
#include "ServerUser.h"
#include "Hotspot.h"

#include <memory.h>
#include <iostream>
#include <fstream>

enum
{
	// I use this ISceneNode ID to indicate a scene node that is
	// not pickable by getSceneNodeAndCollisionPointFromRay()
	ID_IsNotPickable = 0,

	// I use this flag in ISceneNode IDs to indicate that the
	// scene node can be picked by ray selection.
	IDFlag_IsPickable = 1 << 0,
	
	// I use this flag in ISceneNode IDs to indicate that the
	// scene node can be highlighted.  In this example, the
	// homonids can be highlighted, but the level mesh can't.
	IDFlag_IsHighlightable = 1 << 1
};

using namespace irr;

ServerUser::ServerUser(Server *server, R3D::TCPSocket *socket)
	:_server(server), _socket(socket), _device(NULL), _device_thread(NULL),
	_current_frame(NULL), _receive_thread(NULL), _memory_file(NULL), _need_update(NULL),
	_scene_map(NULL), _hotspot_root(NULL)
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

	bool success = true;
	const int length = 1024;
	char buffer[length];

	while (true)
	{
		if (!socket->receive(buffer, length)) 
		{
			//disconnect the client
			ServerInstance->disconnect(client);
			break;
		}

		client->handleCommand(buffer);
	}

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

	std::wstring name;
	
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
	if(!_device->getVideoDriver()->writeImageToFile(_current_frame, _memory_file, 80))
	{
		puts("failed to transfer video frame");
	}

	int length = (int) _memory_file->getPos();

	//transfer screenshot length
	R3D::Packet packet(R3D::UpdateVideoFrame);
	packet.args[0] = length;
	sendPacket(packet);

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
	static wchar_t str[16];
	const R3D::IP &address = getIp();
	swprintf(str, L"%d.%d.%d.%d", address.ip1, address.ip2, address.ip3, address.ip4);
	wstr = str;
}

void ServerUser::handleCommand(const char *cmd)
{
	R3D::Packet packet(cmd);
	std::map<R3D::Command, Callback>::iterator iter = _callbacks.find(packet.command);
	if(iter != _callbacks.end())
	{
		Callback func = iter->second;
		if(func != NULL){
			(this->*func)(packet.args);
			return;
		}
	}
	std::cout << "invalid packet (" << getIp() << "):" << cmd;
}
