#include "ServerUser.h"
#include "Hotspot.h"
#include "Server.h"

using namespace irr;

std::map<R3D::Command, ServerUser::Callback> ServerUser::mCallbacks;

ServerUser::CallbackAdder::CallbackAdder()
{
	if (mCallbacks.empty()) {
		mCallbacks[R3D::CreateDevice] = &ServerUser::createDeviceCommand;
		mCallbacks[R3D::RotateCamera] = &ServerUser::rotateCameraCommand;
		mCallbacks[R3D::ScaleCamera] = &ServerUser::scaleCameraCommand;
		mCallbacks[R3D::MoveCamera] = &ServerUser::moveCameraCommand;
		mCallbacks[R3D::ControlHotspots] = &ServerUser::controlHotspotsCommand;
		mCallbacks[R3D::DoubleClick] = &ServerUser::doubleClickCommand;
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

	ReleaseSemaphore(mNeedUpdate, 1, NULL);
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

	ReleaseSemaphore(mNeedUpdate, 1, NULL);
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

	ReleaseSemaphore(mNeedUpdate, 1, NULL);
}

void ServerUser::controlHotspotsCommand(const Json::Value &)
{
	if (mHotspots.empty()) {
		createHotspots();
	} else {
		clearHotspots();
	}

	ReleaseSemaphore(mNeedUpdate, 1, NULL);
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
	for (std::list<Hotspot *>::iterator i = mHotspots.begin(); i != mHotspots.end(); i++) {
		Hotspot *&hotspot = *i;
		if (hotspot->getNode() == clicked) {
			enterHotspot(hotspot);
			break;
		}
	}
}
