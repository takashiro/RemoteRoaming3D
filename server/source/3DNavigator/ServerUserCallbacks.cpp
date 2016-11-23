#include "ServerUser.h"
#include "Hotspot.h"
#include "Server.h"

using namespace irr;

std::map<R3D::Command, ServerUser::Callback> ServerUser::_callbacks;

ServerUser::CallbackAdder::CallbackAdder()
{
	if(_callbacks.empty())
	{
		_callbacks[R3D::CreateDevice] = &ServerUser::_createDevice;
		_callbacks[R3D::RotateCamera] = &ServerUser::_rotateCamera;
		_callbacks[R3D::ScaleCamera] = &ServerUser::_scaleCamera;
		_callbacks[R3D::MoveCamera] = &ServerUser::_moveCamera;
		_callbacks[R3D::ControlHotspots] = &ServerUser::_controlHotspots;
		_callbacks[R3D::DoubleClick] = &ServerUser::_doubleClick;
	}
}

ServerUser::CallbackAdder adder;

void ServerUser::_createDevice(const Json::Value &args){
	if(_device != NULL){
		return;
	}

	_screen_width = args[0].asInt();
	_screen_height = args[1].asInt();
	_scene_map = ServerInstance->getSceneMapAt(0);

	_memory_file = new irr::io::MemoryFile("screenshot.jpg", _screen_width * _screen_height * 4);
	
	_device_thread = CreateThread(NULL, 0, _DeviceThread, (LPVOID) this, 0, NULL);
}

void ServerUser::_rotateCamera(const Json::Value &args){
	if(_device == NULL)
	{
		return;
	}

	static bool firstUpdate = true;
	static f32 MaxVerticalAngle = 88.0f;
	static f32 RotateSpeed = 0.05f;

	scene::ICameraSceneNode *camera = _device->getSceneManager()->getActiveCamera();
	if(camera == NULL)
	{
		return;
	}

	if (firstUpdate)
	{
		camera->updateAbsolutePosition();
		firstUpdate = false;
	}

	// update position
	core::vector3df pos = camera->getPosition();

	// Update rotation
	core::vector3df target = (camera->getTarget() - camera->getAbsolutePosition());
	core::vector3df relativeRotation = target.getHorizontalAngle();
	core::vector2df CursorPos(float(args[0].asInt64() / 100.0), float(args[1].asInt64() / 100.0));

	relativeRotation.Y -= CursorPos.X * RotateSpeed;
	relativeRotation.X -= CursorPos.Y * RotateSpeed;

	// X < MaxVerticalAngle or X > 360-MaxVerticalAngle
	if (relativeRotation.X > MaxVerticalAngle * 2 && relativeRotation.X < 360.0f - MaxVerticalAngle)
	{
		relativeRotation.X = 360.0f - MaxVerticalAngle;
	}
	else if (relativeRotation.X > MaxVerticalAngle && relativeRotation.X < 360.0f - MaxVerticalAngle)
	{
		relativeRotation.X = MaxVerticalAngle;
	}

	// set target
	core::matrix4 mat;
	mat.setRotationDegrees(core::vector3df(relativeRotation.X, relativeRotation.Y, 0));
	target.set(0,0, core::max_(1.f, pos.getLength()));
	mat.transformVect(target);

	// write translation
	camera->setPosition(pos);

	// write right target
	target += pos;
	camera->setTarget(target);

	ReleaseSemaphore(_need_update, 1, NULL);
}

void ServerUser::_scaleCamera(const Json::Value &args)
{
	if(_device == NULL)
	{
		return;
	}

	scene::ICameraSceneNode *camera = _device->getSceneManager()->getActiveCamera();
	if(camera == NULL)
	{
		return;
	}

	core::vector3df target = camera->getTarget();
	core::vector3df position = camera->getPosition();
	
	core::vector3df look_at = target - position;
	look_at.normalize();
	f32 delta = float(args[0].asInt64() / 1000.0);
	look_at *= delta;

	position += look_at;
	target += look_at;
	camera->setTarget(target);
	camera->setPosition(position);

	ReleaseSemaphore(_need_update, 1, NULL);
}

void ServerUser::_moveCamera(const Json::Value &args){
	if(_device == NULL)
		return;

	float deltaX = args[0].asFloat();
	float deltaY = args[1].asFloat();

	scene::ICameraSceneNode *camera = _device->getSceneManager()->getActiveCamera();
	core::vector3df pos = camera->getPosition();
	core::vector3df target = camera->getTarget();
	core::vector3df look_at = target - pos;
	look_at.normalize();

	pos.X += deltaX;
	pos.Y += deltaY;

	camera->setPosition(pos);

	ReleaseSemaphore(_need_update, 1, NULL);
}

void ServerUser::_controlHotspots(const Json::Value &)
{
	if(_hotspots.empty())
	{
		createHotspots();
	}
	else
	{
		clearHotspots();
	}

	ReleaseSemaphore(_need_update, 1, NULL);
}

void ServerUser::_doubleClick(const Json::Value &args)
{
	if(_device == NULL)
		return;

	core::vector2d<irr::s32> pos;
	pos.X = args[0].asInt();
	pos.Y = args[1].asInt();

	scene::ISceneManager *smgr = _device->getSceneManager();
	scene::ICameraSceneNode *camera = smgr->getActiveCamera();
	if(camera == NULL)
		return;

	//Find the clicked object
	scene::ISceneCollisionManager *cmgr = smgr->getSceneCollisionManager();
	scene::ISceneNode *clicked = cmgr->getSceneNodeFromScreenCoordinatesBB(pos, 0, true, _hotspot_root);

	//Find the corresponding hotspot
	for(std::list<Hotspot *>::iterator i = _hotspots.begin(); i != _hotspots.end(); i++)
	{
		Hotspot *&hotspot = *i;
		if(hotspot->getNode() == clicked)
		{
			enterHotspot(hotspot);
			break;
		}
	}
}
