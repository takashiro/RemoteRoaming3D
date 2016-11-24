#include "ServerUser.h"
#include "Hotspot.h"
#include "Server.h"
#include "Semaphore.h"

using namespace irr;

std::map<Command, ServerUser::Callback> ServerUser::mCallbacks;

ServerUser::CallbackAdder::CallbackAdder()
{
	if (mCallbacks.empty()) {
		mCallbacks[CreateDevice] = &ServerUser::createDeviceCommand;
		mCallbacks[RotateCamera] = &ServerUser::rotateCameraCommand;
		mCallbacks[ScaleCamera] = &ServerUser::scaleCameraCommand;
		mCallbacks[MoveCamera] = &ServerUser::moveCameraCommand;
		mCallbacks[ControlHotspots] = &ServerUser::controlHotspotsCommand;
		mCallbacks[DoubleClick] = &ServerUser::doubleClickCommand;
		mCallbacks[ListMap] = &ServerUser::listMapCommand;
	}
}
ServerUser::CallbackAdder adder;

void ServerUser::rotateCameraCommand(const Json::Value &args)
{
	if (mDevice == nullptr) {
		return;
	}

	static bool firstUpdate = true;
	static f32 MaxVerticalAngle = 88.0f;
	static f32 RotateSpeed = 0.05f;

	scene::ICameraSceneNode *camera = mDevice->getSceneManager()->getActiveCamera();
	if (camera == nullptr) {
		return;
	}

	if (firstUpdate) {
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
	if (relativeRotation.X > MaxVerticalAngle * 2 && relativeRotation.X < 360.0f - MaxVerticalAngle) {
		relativeRotation.X = 360.0f - MaxVerticalAngle;
	} else if (relativeRotation.X > MaxVerticalAngle && relativeRotation.X < 360.0f - MaxVerticalAngle) {
		relativeRotation.X = MaxVerticalAngle;
	}

	// set target
	core::matrix4 mat;
	mat.setRotationDegrees(core::vector3df(relativeRotation.X, relativeRotation.Y, 0));
	target.set(0, 0, core::max_(1.f, pos.getLength()));
	mat.transformVect(target);

	// write translation
	camera->setPosition(pos);

	// write right target
	target += pos;
	camera->setTarget(target);

	mNeedUpdate->release();
}

void ServerUser::scaleCameraCommand(const Json::Value &args)
{
	if (mDevice == nullptr) {
		return;
	}

	scene::ICameraSceneNode *camera = mDevice->getSceneManager()->getActiveCamera();
	if (camera == nullptr) {
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

	mNeedUpdate->release();
}

void ServerUser::moveCameraCommand(const Json::Value &args)
{
	if (mDevice == nullptr)
		return;

	float deltaX = args[0].asFloat();
	float deltaY = args[1].asFloat();

	scene::ICameraSceneNode *camera = mDevice->getSceneManager()->getActiveCamera();
	core::vector3df pos = camera->getPosition();
	core::vector3df target = camera->getTarget();
	core::vector3df look_at = target - pos;
	look_at.normalize();

	pos.X += deltaX;
	pos.Y += deltaY;

	camera->setPosition(pos);

	mNeedUpdate->release();
}

void ServerUser::controlHotspotsCommand(const Json::Value &)
{
	if (mHotspots.empty()) {
		createHotspots();
	} else {
		clearHotspots();
	}

	mNeedUpdate->release();
}

void ServerUser::doubleClickCommand(const Json::Value &args)
{
	if (mDevice == nullptr)
		return;

	core::vector2d<irr::s32> pos;
	pos.X = args[0].asInt();
	pos.Y = args[1].asInt();

	scene::ISceneManager *smgr = mDevice->getSceneManager();
	scene::ICameraSceneNode *camera = smgr->getActiveCamera();
	if (camera == nullptr)
		return;

	//Find the clicked object
	scene::ISceneCollisionManager *cmgr = smgr->getSceneCollisionManager();
	scene::ISceneNode *clicked = cmgr->getSceneNodeFromScreenCoordinatesBB(pos, 0, true, mHotspotRoot);

	//Find the corresponding hotspot
	for (Hotspot *hotspot : mHotspots) {
		if (hotspot->getNode() == clicked) {
			enterHotspot(hotspot);
			break;
		}
	}
}

void ServerUser::listMapCommand(const Json::Value &)
{
	const std::vector<SceneMap *> &maps = mServer->getSceneMaps();
	Json::Value results(Json::arrayValue);
	int id = 0;
	for (SceneMap *map : maps) {
		Json::Value info(Json::objectValue);
		info["id"] = id++;
		info["name"] = map->name;
		info["description"] = map->description;
		results.append(std::move(info));
	}

	Packet packet(ListMap);
	packet.args = std::move(results);
	sendPacket(packet);
}
