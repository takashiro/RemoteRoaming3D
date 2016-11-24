#include "Server.h"
#include "ServerUser.h"
#include "Hotspot.h"
#include "Thread.h"
#include "util.h"

#include <memory.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace irr;

RD_NAMESPACE_BEGIN

ServerUser::ServerUser(Server *server, R3D::TCPSocket *socket)
	: mServer(server)
	, mSocket(socket)
	, mDevice(nullptr)
	, mDeviceThread(nullptr)
	, mCurrentFrame(nullptr)
	, mReceiveThread(nullptr)
	, mMemoryFile(nullptr)
	, mNeedUpdate(nullptr)
	, mSceneMap(nullptr)
	, mHotspotRoot(nullptr)
	, mPacketHandler(&ServerUser::handleConnection)
	, mReadSocket(&ServerUser::readLine)
{
}

ServerUser::~ServerUser()
{
	disconnect();

	if (mNeedUpdate) {
		CloseHandle(mNeedUpdate);
	}

	if (mMemoryFile) {
		mMemoryFile->drop();
	}
}

void ServerUser::readLine(char *buffer, int buffer_capacity, int length)
{
	char *buffer_end = buffer + length;
	char *begin = buffer;
	char *end = nullptr;
	for (;;) {
		//find the end of the current line
		end = begin;
		while (*end != '\n' && *end != '\r' && end < buffer_end) {
			end++;
		}

		if (*end == '\n' || *end == '\r') {
			//we find a whole line, handle it
			if (*end == '\r') {
				*end = '\0';
				if (end + 1 < buffer_end) {
					if (end[1] == '\n') {
						end++;
						*end = '\0';
					}
				}
			} else {
				*end = '\0';
			}
			(this->*mPacketHandler)(begin);
			begin = end + 1;

			//if there's no more lines, go to receive data again
			if (begin >= buffer_end) {
				break;
			}
		} else {
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
	} else if (payload_length == 127) {
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
	} else {
		//Unsupported yet
	}
}

void ServerUser::sendPacket(const R3D::Packet &packet)
{
	char header[10] = { '\x81', '\0' };

	std::string data = packet.toString();
	size_t length = data.length();
	if (length < 126) {
		header[1] = static_cast<char>(length);
		mSocket->write(header, 2);
	} else if (length <= 0xFFFF) {
		header[1] = 126;
		const char *bytes = reinterpret_cast<const char *>(&length);
		header[2] = bytes[1];
		header[3] = bytes[0];
		mSocket->write(header, 4);
	} else {
		header[1] = 127;
		const char *bytes = reinterpret_cast<const char *>(&length);
		for (int i = 0; i < 8; i++) {
			header[2 + i] = bytes[7 - i];
		}
		mSocket->write(header, 10);
	}
	mSocket->write(data);
}

void ServerUser::disconnect()
{
	mSocket->close();

	if (mDevice) {
		mDevice->closeDevice();
		ReleaseSemaphore(mNeedUpdate, 1, NULL);
	}

	if (mDeviceThread) {
		mDeviceThread->wait();
		delete mDeviceThread;
		mDeviceThread = nullptr;
	}

	if (mReceiveThread) {
		mReceiveThread->wait();
		delete mReceiveThread;
		mReceiveThread = nullptr;
	}
}

void ServerUser::startService()
{
	mReceiveThread = new Thread([this]() {
		static int buffer_capacity = 8 * 1024;
		char *buffer = new char[buffer_capacity + 1];
		int length = 0;

		while ((length = mSocket->read(buffer, buffer_capacity)) > 0) {
			(this->*(mReadSocket))(buffer, buffer_capacity, length);
		}

		//disconnect the client
		mServer->disconnect(this);

		delete[] buffer;
	});
	mNeedUpdate = CreateSemaphore(NULL, 1, 1, NULL);
}

void ServerUser::createHotspots()
{
	if (!mHotspots.empty() || mSceneMap == NULL)
		return;

	if (mSceneMap->hotspotPath.empty())
		return;

	char buffer[3];
	std::ifstream hotspot_file(mSceneMap->hotspotPath, std::ios::binary);
	hotspot_file.read(buffer, 3);

	//Handle UTF-8 BOM
	if (buffer[0] != '\xEF' || buffer[1] != '\xBB' || buffer[2] != '\xBF') {
		hotspot_file.putback(buffer[2]);
		hotspot_file.putback(buffer[1]);
		hotspot_file.putback(buffer[0]);
	}

	Json::Value hotspots;
	Json::Reader reader;
	bool success = reader.parse(hotspot_file, hotspots);
	hotspot_file.close();
	if (!success)
		return;

	scene::ISceneManager *smgr = mDevice->getSceneManager();
	mHotspotRoot = smgr->addEmptySceneNode();

	for (const Json::Value &hotspot : hotspots) {
		Hotspot *spot = new Hotspot(hotspot);
		std::wstring name = convert_string(spot->getName());
		scene::IBillboardTextSceneNode *head_text = smgr->addBillboardTextSceneNode(0, name.c_str(), mHotspotRoot, spot->getSize(), spot->getPosition());
		spot->setNode(head_text);
		mHotspots.push_back(spot);
	}
}

void ServerUser::clearHotspots()
{
	if (mHotspotRoot == NULL || mHotspots.empty())
		return;

	static core::vector3df far_away(FLT_MIN, FLT_MIN, FLT_MIN);
	scene::ISceneManager *smgr = mDevice->getSceneManager();
	for (std::list<Hotspot *>::iterator i = mHotspots.begin(); i != mHotspots.end(); i++) {
		Hotspot *&spot = *i;
		scene::ISceneNode *node = spot->getNode();
		node->setPosition(far_away);
		delete spot;
	}

	mHotspots.clear();
	smgr->addToDeletionQueue(mHotspotRoot);
	mHotspotRoot = NULL;
}

void ServerUser::sendScreenshot()
{
	if (mDevice == NULL || mCurrentFrame == NULL) {
		return;
	}

	mMemoryFile->clear();
	if (!mDevice->getVideoDriver()->writeImageToFile(mCurrentFrame, mMemoryFile, 50)) {
		puts("failed to transfer video frame");
	}

	int length = (int)mMemoryFile->getPos();

	std::string raw(mMemoryFile->getContent(), length);
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
		mPacketHandler = &ServerUser::handleCommand;
		handleCommand(cmd);
	} else {
		if (strnicmp(cmd, "GET ", 4) != 0) {
			mPacketHandler = &ServerUser::handleWebSocket;
		}
	}
}

void ServerUser::handleWebSocket(const char *cmd)
{
	if (strnicmp(cmd, "Sec-WebSocket-Key: ", 19) == 0) {
		std::string sec_key = cmd + 19;
		sec_key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

		std::string key_accept = base64_encode(sha1(sec_key));

		mSocket->write("HTTP/1.1 101 Switching Protocols\n");
		mSocket->write("Upgrade: websocket\n");
		mSocket->write("Connection: Upgrade\n");
		mSocket->write("Sec-WebSocket-Accept: ");
		mSocket->write(key_accept);
		mSocket->write("\n\n");
	} else if (*cmd == '\0') {
		mPacketHandler = &ServerUser::handleCommand;
		mReadSocket = &ServerUser::readFrame;
	}
}

void ServerUser::handleCommand(const char *cmd)
{
	if (*cmd != '[') {
		return;
	}

	R3D::Packet packet(cmd);
	std::map<R3D::Command, Callback>::iterator iter = mCallbacks.find(packet.command);
	if (iter != mCallbacks.end()) {
		Callback func = iter->second;
		if (func) {
			(this->*func)(packet.args);
			return;
		}
	}
	std::cout << "invalid packet (" << getIp() << "):" << cmd;
}

RD_NAMESPACE_END
