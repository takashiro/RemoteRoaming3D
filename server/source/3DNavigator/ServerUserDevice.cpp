#include "Server.h"
#include "ServerUser.h"
#include "Thread.h"
#include "Semaphore.h"

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

#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_ERASEBKGND:
	return 0;

	case WM_DESTROY:
	PostQuitMessage(0);
	return 0;

	case WM_SYSCOMMAND:
	// prevent screensaver or monitor powersave mode from starting
	if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
		(wParam & 0xFFF0) == SC_MONITORPOWER ||
		(wParam & 0xFFF0) == SC_KEYMENU
		)
		return 0;

	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
};

#endif //if compiled with windows

void ServerUser::createDeviceCommand(const Json::Value &args)
{
	if (mDevice) {
		mDevice->closeDevice();
		mClosingDevice = true;
		mNeedUpdate->release();
		mDeviceThread->wait();
		mDevice->drop();
		delete mDeviceThread;
	}

	mScreenWidth = args[0].asInt();
	mScreenHeight = args[1].asInt();
	mSceneMap = mServer->getSceneMapAt(args[2].asInt());

	mMemoryFile = new irr::io::MemoryFile("screenshot.jpg", mScreenWidth * mScreenHeight * 4);

	std::function<void()> worker = [this]() {
		if (mSceneMap == nullptr)
			return;

		//set up creation parameters
		SIrrlichtCreationParameters p;
		p.DriverType = mServer->getDriverType();
		p.WindowSize = core::dimension2d<u32>(mScreenWidth, mScreenHeight);
		p.Bits = (u8)16U;
		p.Fullscreen = false;
		p.Stencilbuffer = false;
		p.Vsync = false;
		p.EventReceiver = NULL;
		p.LoggingLevel = ELL_NONE;

#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
		//create a default hidden window to render the scene
		const fschar_t* ClassName = __TEXT("CIrrDeviceWin32");

		// Register Class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandle(0);
		wcex.hIcon = NULL;
		wcex.hCursor = 0;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = ClassName;
		wcex.hIconSm = 0;

		// if there is an icon, load it
		wcex.hIcon = (HICON)LoadImage(wcex.hInstance, __TEXT("irrlicht.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

		RegisterClassEx(&wcex);

		// calculate client size
		RECT clientSize;
		clientSize.top = 0;
		clientSize.left = 0;
		clientSize.right = mScreenWidth;
		clientSize.bottom = mScreenHeight;

		DWORD style = WS_POPUP;//WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		AdjustWindowRect(&clientSize, style, FALSE);

		const s32 realWidth = clientSize.right - clientSize.left;
		const s32 realHeight = clientSize.bottom - clientSize.top;

		s32 windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
		s32 windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

		if (windowLeft < 0)
			windowLeft = 0;
		if (windowTop < 0)
			windowTop = 0;	// make sure window menus are in screen on creation

							// create window
		HWND HWnd = CreateWindow(ClassName, __TEXT(""), style, windowLeft, windowTop, realWidth, realHeight, NULL, NULL, wcex.hInstance, NULL);

		ShowWindow(HWnd, SW_HIDE);
		UpdateWindow(HWnd);

		// fix ugly ATI driver bugs. Thanks to ariaci
		MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);

		p.WindowId = HWnd;
#endif

		mDevice = createDeviceEx(p);
		if (mDevice == nullptr) {
			return; // could not create selected driver.
		}

		/*
		Get a pointer to the video driver and the SceneManager so that
		we do not always have to call irr::IrrlichtDevice::getVideoDriver() and
		irr::IrrlichtDevice::getSceneManager().
		*/
		video::IVideoDriver* driver = mDevice->getVideoDriver();
		scene::ISceneManager* smgr = mDevice->getSceneManager();

		/*
		To display the Quake 3 map, we first need to load it. Quake 3 maps
		are packed into .pk3 files which are nothing else than .zip files.
		So we add the .pk3 file to our irr::io::IFileSystem. After it was added,
		we are able to read from the files in that archive as if they are
		directly stored on the disk.
		*/
		mDevice->getFileSystem()->addFileArchive(mSceneMap->path.c_str());

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
		scene::IAnimatedMesh* mesh = smgr->getMesh(mSceneMap->meshPath.c_str());
		scene::IMeshSceneNode* node = nullptr;

		if (mesh) {
			node = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);

			if (node) {
				node->setMaterialFlag(irr::video::EMF_LIGHTING, true);

				node->setScale(core::vector3df(80, 80, 80));
				node->setRotation(core::vector3df(-90, 0, 0));
				node->setPosition(core::vector3df(0, -40, 0));

				node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);

				scene::ILightSceneNode *light = smgr->addLightSceneNode(0, core::vector3df(0, 10, 5), video::SColorf(1.0f, 1.0f, 1.0f), 100);
				smgr->setAmbientLight(video::SColor(0, 160, 160, 160));
				light->setPosition(core::vector3df(0, 10, 10));
			}
		}

		/*
		Now we only need a camera to look at the Quake 3 map.
		Set a jump speed of 3 units per second, which gives a fairly realistic jump
		when used with the gravity of (0, -10, 0) in the collision response animator.
		*/
		scene::ICameraSceneNode* camera = smgr->addCameraSceneNode(0, mSceneMap->cameraPosition, mSceneMap->cameraTarget);
		camera->setAspectRatio(static_cast<irr::f32>(mScreenWidth) / mScreenHeight);

		mClosingDevice = false;
		while (mDevice->run()) {
			mNeedUpdate->acquire();
			if (mClosingDevice) {
				break;
			}
			driver->beginScene(true, true, video::SColor(255, 200, 200, 200));
			smgr->drawAll();
			driver->endScene();

			if (mCurrentFrame) {
				mCurrentFrame->drop();
			}
			mCurrentFrame = driver->createScreenShot();
			sendScreenshot();
		}

		mDevice->drop();
	};

	mDeviceThread = new Thread(std::move(worker));
	mNeedUpdate->release();
}
