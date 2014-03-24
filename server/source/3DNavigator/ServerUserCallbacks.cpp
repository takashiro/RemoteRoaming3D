#include "ServerUser.h"
#include "Hotspot.h"

using namespace irr;

void ServerUser::_createDevice(const Json::Value &args){
	if(_device != NULL){
		return;
	}

	_screen_width = args[0].asInt();
	_screen_height = args[1].asInt();
	
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
	core::vector2df CursorPos(args[0].asFloat(), args[1].asFloat());

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
	f32 delta = args[0].asFloat() * 0.1f;
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

	float pos_x = args[0].asFloat();
	float pos_y = args[1].asFloat();
	static f32 max_distance = 60.0f;
	
	scene::ICameraSceneNode *camera = _device->getSceneManager()->getActiveCamera();
	if(camera == NULL)
		return;

	//Find the nearest hotspot
	core::vector3df pos = camera->getPosition();
	Hotspot *nearest = NULL;
	std::list<Hotspot *>::iterator i;
	f32 nearest_distance = FLT_MAX;
	for(i = _hotspots.begin(); i != _hotspots.end(); i++)
	{
		scene::ISceneNode *text = (*i)->getNode();
		f32 distance = text->getPosition().getDistanceFrom(pos);
		if(distance <= max_distance)
		{
			nearest = *i;
			nearest_distance = distance;
			break;
		}
	}
	
	if(nearest == NULL)
		return;

	for(; i != _hotspots.end(); i++)
	{
		scene::ISceneNode *text = (*i)->getNode();
		f32 distance = text->getPosition().getDistanceFrom(pos);
		if(distance <= max_distance)
		{
			if(distance < nearest_distance)
			{
				nearest = *i;
				nearest_distance = distance;
			}
		}
	}

	//Enter the hotspot
	enterHotspot(nearest);
}
