#include "Server.h"
#include "ServerUser.h"
#include "IrrMemoryFile.h"
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

std::map<R3D::Command, ServerUser::Callback> ServerUser::_callbacks;

ServerUser::ServerUser(Server *server, SOCKET socket)
	:_server(server), _socket(socket), _device(NULL), _device_thread(NULL),
	_current_frame(NULL)
{
	if(_callbacks.empty())
	{
		_callbacks[R3D::SetResolution] = &ServerUser::_createDevice;
		_callbacks[R3D::RotateCamera] = &ServerUser::_rotateCamera;
		_callbacks[R3D::ScaleCamera] = &ServerUser::_scaleCamera;
		_callbacks[R3D::MoveCamera] = &ServerUser::_moveCamera;
		_callbacks[R3D::ControlHotspots] = &ServerUser::_controlHotspots;
		_callbacks[R3D::DoubleClick] = &ServerUser::_doubleClick;
	}

	_receive_thread = CreateThread(NULL, 0, _ReceiveThread, (LPVOID) this, 0, NULL);
	_need_update = CreateSemaphore(NULL, 1, 1, NULL);
	_is_sending_data = CreateSemaphore(NULL, 1, 1, NULL);
}

ServerUser::~ServerUser(){
	closesocket(_socket);

	if(_receive_thread != NULL){
		CloseHandle(_receive_thread);
	}

	if(_device_thread != NULL){
		CloseHandle(_device_thread);
	}

	CloseHandle(_need_update);
}

DWORD WINAPI ServerUser::_ReceiveThread(LPVOID pParam){
	ServerUser *client = (ServerUser *) pParam;
	SOCKET &socket = client->_socket;

	int result = 0;
	const int length = 1024;
	char buffer[length];

	while (true)
	{
		result = recv(socket, buffer, length, 0);
		
		if (result == 0 || result == SOCKET_ERROR) 
		{
			//disconnect the client
			client->disconnect();
			delete client;
			break;
		}

		client->handleCommand(buffer);
	}

	return 0;
}

void ServerUser::sendPacket(const std::string &raw)
{
	WaitForSingleObject(_is_sending_data, INFINITE);

	size_t length = (int) raw.length();
	size_t y = 0;
	const char *p = raw.c_str();
	while(y < length)
	{
		y += send(_socket, p + y, int(length - y), 0);
	}

	ReleaseSemaphore(_is_sending_data, 1, NULL);
}

void ServerUser::disconnect()
{
	if(_device){
		_device->closeDevice();
		ReleaseSemaphore(_need_update, 1, NULL);
	}
	WaitForSingleObject(_device_thread, INFINITE);
	_server->disconnect(this);
}

void ServerUser::createHotspots()
{
	if(!_hotspots.empty())
		return;

	std::ifstream hotspot_file("../../media/158.txt");
	Json::Value hotspots;
	Json::Reader reader;
	bool success = reader.parse(hotspot_file, hotspots);
	hotspot_file.close();
	if(!success)
		return;

	scene::ISceneManager *smgr = _device->getSceneManager();
	for(Json::Value::iterator i = hotspots.begin(); i != hotspots.end(); i++)
	{
		Hotspot *spot = new Hotspot(*i);
		scene::IBillboardTextSceneNode *head_text = smgr->addBillboardTextSceneNode(0, str2wstr(spot->getName()).c_str(), 0, spot->getSize(), spot->getPosition());
		spot->setNode(head_text);
		_hotspots.push_back(spot);
	}
}

void ServerUser::clearHotspots()
{
	if(_hotspots.empty())
		return;

	static core::vector3df far_away(FLT_MIN, FLT_MIN, FLT_MIN);
	scene::ISceneManager *smgr = _device->getSceneManager();
	for(std::list<Hotspot *>::iterator i = _hotspots.begin(); i != _hotspots.end(); i++)
	{
		Hotspot *&spot = *i;
		scene::ISceneNode *node = spot->getNode();
		node->setPosition(far_away);
		smgr->addToDeletionQueue(node);
		delete spot;
	}

	_hotspots.clear();
}

void ServerUser::sendScreenshot()
{
	if(_device == NULL || _current_frame == NULL)
	{
		return;
	}

	IrrMemoryFile *file = new IrrMemoryFile("screenshot.jpg");
	if(!_device->getVideoDriver()->writeImageToFile(_current_frame, file))
	{
		puts("failed to transfer video frame");
	}

	const std::string &content = file->getContent();
	size_t length = content.size();

	//transfer screenshot length
	R3D::Packet packet(R3D::UpdateVideoFrame);
	packet.args[0] = length;
	sendPacket(packet);

	//transfer the picture
	sendPacket(content);

	file->drop();
}

sockaddr_in ServerUser::getIp() const
{
	sockaddr_in ip;
	int length = sizeof(ip);
	getpeername(_socket, (sockaddr *) &ip, &length);
	return ip;
}

void ServerUser::getIp(std::wstring &wstr)
{
	const sockaddr_in &ip = getIp();
	wchar_t str[16];
	swprintf(str, L"%d.%d.%d.%d", ip.sin_addr.S_un.S_un_b.s_b1, ip.sin_addr.S_un.S_un_b.s_b2, ip.sin_addr.S_un.S_un_b.s_b3, ip.sin_addr.S_un.S_un_b.s_b4);
	wstr = str;
}

void ServerUser::handleCommand(const char *cmd)
{
	R3D::Packet packet = R3D::Packet::FromString(cmd);
	std::map<R3D::Command, Callback>::iterator iter = _callbacks.find(packet.command);
	if(iter != _callbacks.end())
	{
		Callback func = iter->second;
		if(func != NULL){
			(this->*func)(packet.args);
			return;
		}
	}
	printf("invalid packet:");
	puts(cmd);
}

DWORD WINAPI ServerUser::_DeviceThread(LPVOID lpParam){
	ServerUser *client = (ServerUser *) lpParam;
	
	IrrlichtDevice *&device = client->_device;

	device = createDevice(ServerInstance->getDriverType(), core::dimension2d<u32>(client->_screen_width, client->_screen_height));	
	if (device == NULL)
		return 1; // could not create selected driver.

	/*
	Get a pointer to the video driver and the SceneManager so that
	we do not always have to call irr::IrrlichtDevice::getVideoDriver() and
	irr::IrrlichtDevice::getSceneManager().
	*/
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	//hide the window
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
	ShowWindow(reinterpret_cast<HWND>(driver->getExposedVideoData().OpenGLWin32.HWnd), SW_HIDE);
#endif

	//Disable all logs
	device->getLogger()->setLogLevel(irr::ELL_NONE);

	/*
	To display the Quake 3 map, we first need to load it. Quake 3 maps
	are packed into .pk3 files which are nothing else than .zip files.
	So we add the .pk3 file to our irr::io::IFileSystem. After it was added,
	we are able to read from the files in that archive as if they are
	directly stored on the disk.
	*/

	device->getFileSystem()->addFileArchive("../../media/158.zip");
	
	/*
	Now we can load the mesh by calling
	irr::scene::ISceneManager::getMesh(). We get a pointer returned to an
	irr::scene::IAnimatedMesh. As you might know, Quake 3 maps are not
	really animated, they are only a huge chunk of static geometry with
	some materials attached. Hence the IAnimatedMesh consists of only one
	frame, so we get the "first frame" of the "animation", which is our
	quake level and create an Octree scene node with it, using
	irr::scene::ISceneManager::addOctreeSceneNode().
	The Octree optimizes the scene a little bit, trying to draw only geometry
	which is currently visible. An alternative to the Octree would be a
	irr::scene::IMeshSceneNode, which would always draw the complete
	geometry of the mesh, without optimization. Try it: Use
	irr::scene::ISceneManager::addMeshSceneNode() instead of
	addOctreeSceneNode() and compare the primitives drawn by the video
	driver. (There is a irr::video::IVideoDriver::getPrimitiveCountDrawn()
	method in the irr::video::IVideoDriver class). Note that this
	optimization with the Octree is only useful when drawing huge meshes
	consisting of lots of geometry.
	*/
	scene::IAnimatedMesh* mesh = smgr->getMesh("MG158_52.obj");
	scene::IMeshSceneNode* node = 0;

	if (mesh)
		node = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);

	/*
	Because the level was not modelled around the origin (0,0,0), we
	translate the whole level a little bit. This is done on
	irr::scene::ISceneNode level using the methods
	irr::scene::ISceneNode::setPosition() (in this case),
	irr::scene::ISceneNode::setRotation(), and
	irr::scene::ISceneNode::setScale().
	*/

	scene::ITriangleSelector* selector = 0;
	irr::scene::IBillboardSceneNode * bill = 0;
	irr::scene::ILightSceneNode *light = 0;
	if (node)
	{
		node->setMaterialFlag(irr::video::EMF_LIGHTING,true);

		node->setScale(core::vector3df(80,80,80));
		node->setRotation(core::vector3df(-90,0,0));
		node->setPosition(core::vector3df(0,-40,0));

		node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS,true);
		light = smgr->addLightSceneNode(0,core::vector3df(0,10,5),video::SColorf(1.0f, 1.0f, 1.0f),100);
		smgr->setAmbientLight(video::SColor(0,160,160,160));
		light->setPosition(core::vector3df(0,10,10));
		
		// attach billboard to light
		bill = smgr->addBillboardSceneNode(light, core::dimension2d<f32>(30, 30));
		bill->setMaterialFlag(video::EMF_LIGHTING, false);
		bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
		bill->setMaterialTexture(0, driver->getTexture("../../media/particlewhite.bmp"));

		selector = smgr->createOctreeTriangleSelector(
			node->getMesh(), node, 128);
		node->setTriangleSelector(selector);
		// We're not done with this selector yet, so don't drop it.
	}

	/*
	Now we only need a camera to look at the Quake 3 map.
	Set a jump speed of 3 units per second, which gives a fairly realistic jump
	when used with the gravity of (0, -10, 0) in the collision response animator.
	*/
	scene::ICameraSceneNode* camera = smgr->addCameraSceneNode();
	camera->setPosition(core::vector3df(0,20,-5));
	camera->setFarValue(5000.0f);

	if (selector)
	{
		scene::ISceneNodeAnimator* anim = smgr->createCollisionResponseAnimator(
			selector, camera, core::vector3df(6,10,6),
			core::vector3df(0,0,0), core::vector3df(0,20,0));
		selector->drop(); // As soon as we're done with the selector, drop it.
		camera->addAnimator(anim);
		anim->drop();  // And likewise, drop the animator when we're done referring to it.
	}

	/*
	We have done everything, so lets draw it. We also write the current
	frames per second and the primitives drawn into the caption of the
	window. The test for irr::IrrlichtDevice::isWindowActive() is optional,
	but prevents the engine to grab the mouse cursor after task switching
	when other programs are active. The call to
	irr::IrrlichtDevice::yield() will avoid the busy loop to eat up all CPU
	cycles when the window is not active.
	*/

	video::SMaterial material;
	material.setTexture(0,0);
	material.Lighting = false;
	material.Wireframe = true;
	scene::ISceneNode* highlightedSceneNode = 0;
	scene::ISceneCollisionManager* collMan = smgr->getSceneCollisionManager();

	int lastFPS = -1;

	while(device->run())
	{
		WaitForSingleObject(client->_need_update, INFINITE);
		driver->beginScene(true, true, video::SColor(255,200,200,200));
		smgr->drawAll();

		// All intersections in this example are done with a ray cast out from the camera to
		// a distance of 1000.  You can easily modify this to check (e.g.) a bullet
		// trajectory or a sword's position, or create a ray from a mouse click position using
		// ISceneCollisionManager::getRayFromScreenCoordinates()
		core::line3d<f32> ray;
		ray.start = camera->getPosition();
		ray.end = ray.start + (camera->getTarget() - ray.start).normalize() * 10.0f;

		// Tracks the current intersection point with the level or a mesh
		core::vector3df intersection;
		// Used to show with triangle has been hit
		core::triangle3df hitTriangle;

		// This call is all you need to perform ray/triangle collision on every scene node
		// that has a triangle selector, including the Quake level mesh.  It finds the nearest
		// collision point/triangle, and returns the scene node containing that point.
		// Irrlicht provides other types of selection, including ray/triangle selector,
		// ray/box and ellipse/triangle selector, plus associated helpers.
		// See the methods of ISceneCollisionManager
		scene::ISceneNode * selectedSceneNode =
			collMan->getSceneNodeAndCollisionPointFromRay(
			ray,
			intersection, // This will be the position of the collision
			hitTriangle, // This will be the triangle hit in the collision
			IDFlag_IsPickable, // This ensures that only nodes that we have
			// set up to be pickable are considered
			0); // Check the entire scene (this is actually the implicit default)

		// If the ray hit anything, move the billboard to the collision position
		// and draw the triangle that was hit.
		if(selectedSceneNode)
		{
			//	bill->setPosition(intersection);

			// We need to reset the transform before doing our own rendering.
			driver->setTransform(video::ETS_WORLD, core::matrix4());
			driver->setMaterial(material);
			driver->draw3DTriangle(hitTriangle, video::SColor(0,255,0,0));
		}

		driver->endScene();

		if(client->_current_frame != NULL){
			client->_current_frame->drop();
		}
		client->_current_frame = driver->createScreenShot();
		client->sendScreenshot();

		int fps = driver->getFPS();
		if (lastFPS != fps)
		{
			std::wstring str;
			client->getIp(str);
			str += L" [";
			str += driver->getName();
			str += L"] FPS:";
			str += fps;

			device->setWindowCaption(str.c_str());

			lastFPS = fps;
		}
	}

	/*
	In the end, delete the Irrlicht device.
	*/
	device->drop();

	return 0;
}
