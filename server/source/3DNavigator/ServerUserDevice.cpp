#include "Server.h"
#include "ServerUser.h"

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
	switch (message)
	{
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

DWORD WINAPI ServerUser::_DeviceThread(LPVOID lpParam){
	ServerUser *client = (ServerUser *) lpParam;
	IrrlichtDevice *&device = client->_device;

	//set up creation parameters
	SIrrlichtCreationParameters p;
	p.DriverType = ServerInstance->getDriverType();
	p.WindowSize = core::dimension2d<u32>(client->_screen_width, client->_screen_height);
	p.Bits = (u8) 16U;
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
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(0);
	wcex.hIcon			= NULL;
	wcex.hCursor		= 0;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= ClassName;
	wcex.hIconSm		= 0;

	// if there is an icon, load it
	wcex.hIcon = (HICON)LoadImage(wcex.hInstance, __TEXT("irrlicht.ico"), IMAGE_ICON, 0,0, LR_LOADFROMFILE);

	RegisterClassEx(&wcex);

	// calculate client size
	RECT clientSize;
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = client->_screen_width;
	clientSize.bottom = client->_screen_height;

	DWORD style = WS_POPUP;//WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	AdjustWindowRect(&clientSize, style, FALSE);

	const s32 realWidth = clientSize.right - clientSize.left;
	const s32 realHeight = clientSize.bottom - clientSize.top;

	s32 windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	s32 windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	if ( windowLeft < 0 )
		windowLeft = 0;
	if ( windowTop < 0 )
		windowTop = 0;	// make sure window menus are in screen on creation

	// create window
	HWND HWnd = CreateWindow( ClassName, __TEXT(""), style, windowLeft, windowTop, realWidth, realHeight, NULL, NULL, wcex.hInstance, NULL);

	ShowWindow(HWnd, SW_HIDE);
	UpdateWindow(HWnd);

	// fix ugly ATI driver bugs. Thanks to ariaci
	MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);

	p.WindowId = HWnd;
#endif

	device = createDeviceEx(p);
	if (device == NULL)
		return 1; // could not create selected driver.

	/*
	Get a pointer to the video driver and the SceneManager so that
	we do not always have to call irr::IrrlichtDevice::getVideoDriver() and
	irr::IrrlichtDevice::getSceneManager().
	*/
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

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
