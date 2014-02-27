/** Example 002 Quake3Map

This Tutorial shows how to load a Quake 3 map into the engine, create a
SceneNode for optimizing the speed of rendering, and how to create a user
controlled camera.

Please note that you should know the basics of the engine before starting this
tutorial. Just take a short look at the first tutorial, if you haven't done
this yet: http://irrlicht.sourceforge.net/tut001.html

Lets start like the HelloWorld example: We include the irrlicht header files
and an additional file to be able to ask the user for a driver type using the
console.
*/
#include <irrlicht.h>
#include <iostream>

/*
As already written in the HelloWorld example, in the Irrlicht Engine everything
can be found in the namespace 'irr'. To get rid of the irr:: in front of the
name of every class, we tell the compiler that we use that namespace from now
on, and we will not have to write that 'irr::'. There are 5 other sub
namespaces 'core', 'scene', 'video', 'io' and 'gui'. Unlike in the HelloWorld
example, we do not call 'using namespace' for these 5 other namespaces, because
in this way you will see what can be found in which namespace. But if you like,
you can also include the namespaces like in the previous example.
*/
using namespace irr;

/*
Again, to be able to use the Irrlicht.DLL file, we need to link with the
Irrlicht.lib. We could set this option in the project settings, but to make it
easy, we use a pragma comment lib:
*/
#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

/*
Ok, lets start. Again, we use the main() method as start, not the WinMain().
*/

#define USE_BNG_ADD

#ifdef USE_BNG_ADD
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

#include "socket.h"

#endif

int main()
{
	/*
	Like in the HelloWorld example, we create an IrrlichtDevice with
	createDevice(). The difference now is that we ask the user to select
	which video driver to use. The Software device might be
	too slow to draw a huge Quake 3 map, but just for the fun of it, we make
	this decision possible, too.
	Instead of copying this whole code into your app, you can simply include
	driverChoice.h from Irrlicht's include directory. The function
	driverChoiceConsole does exactly the same.
	*/

	// ask user for driver

	video::E_DRIVER_TYPE driverType;

	printf("Please select the driver you want for this example:\n"\
		" (a) OpenGL 1.5\n (b) Direct3D 9.0c\n (c) Direct3D 8.1\n"\
		" (d) Burning's Software Renderer\n (e) Software Renderer\n"\
		" (f) NullDevice\n (otherKey) exit\n\n");

	char i;
	std::cin >> i;

	switch(i)
	{
		case 'a': driverType = video::EDT_OPENGL;   break;
		case 'b': driverType = video::EDT_DIRECT3D9;break;
		case 'c': driverType = video::EDT_DIRECT3D8;break;
		case 'd': driverType = video::EDT_BURNINGSVIDEO;break;
		case 'e': driverType = video::EDT_SOFTWARE; break;
		case 'f': driverType = video::EDT_NULL;     break;
		default: return 1;
	}

	// create device and exit if creation failed

	IrrlichtDevice *device =
		createDevice(driverType, core::dimension2d<u32>(640, 480));

	if (device == 0)
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
#ifndef USE_BNG_ADD

		device->getFileSystem()->addFileArchive("../../media/map-20kdm2.pk3");
#else
		device->getFileSystem()->addFileArchive("../../media/158.zip");
	//	device->getFileSystem()->addFileArchive("../../bngAdded/sunflower/sunflower.zip");
	//  device->getFileSystem()->addFileArchive("../../bngAdded/Ferrari_f430/Ferrari_f430.zip");
#endif // BNG_ADD
	




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
#ifdef USE_BNG_ADD

	scene::IAnimatedMesh* mesh = smgr->getMesh("MG158_52.obj");
	//	scene::IAnimatedMesh* mesh = smgr->getMesh("SunFlower.obj");
	//	scene::IAnimatedMesh* mesh = smgr->getMesh("f430.obj");
#else
	scene::IAnimatedMesh* mesh = smgr->getMesh("20kdm2.bsp");
#endif


#ifdef USE_BNG_ADD
	scene::IMeshSceneNode* node = 0;
#else
	scene::ISceneNode* node = 0;
#endif
	if (mesh)
		node = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);
//		node = smgr->addMeshSceneNode(mesh->getMesh(0));

	/*
	Because the level was not modelled around the origin (0,0,0), we
	translate the whole level a little bit. This is done on
	irr::scene::ISceneNode level using the methods
	irr::scene::ISceneNode::setPosition() (in this case),
	irr::scene::ISceneNode::setRotation(), and
	irr::scene::ISceneNode::setScale().
	*/
#ifdef USE_BNG_ADD
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

#else
	if (node)
		node->setPosition(core::vector3df(-1300,-144,-1249));
		
#endif

	/*
	Now we only need a camera to look at the Quake 3 map.
	We want to create a user controlled camera. There are some
	cameras available in the Irrlicht engine. For example the
	MayaCamera which can be controlled like the camera in Maya:
	Rotate with left mouse button pressed, Zoom with both buttons pressed,
	translate with right mouse button pressed. This could be created with
	irr::scene::ISceneManager::addCameraSceneNodeMaya(). But for this
	example, we want to create a camera which behaves like the ones in
	first person shooter games (FPS) and hence use
	irr::scene::ISceneManager::addCameraSceneNodeFPS().
	*/
#ifdef USE_BNG_ADD
	// Set a jump speed of 3 units per second, which gives a fairly realistic jump
	// when used with the gravity of (0, -10, 0) in the collision response animator.
	scene::ICameraSceneNode* camera =
		smgr->addCameraSceneNodeFPS(0, 10.0f, .05f, ID_IsNotPickable, 0, 0, true, 3.f);
	camera->setPosition(core::vector3df(0,20,-5));
	camera->setTarget(core::vector3df(0,20,5));
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
#else
	smgr->addCameraSceneNodeFPS(0,10,0.2);
#endif

	/*
	The mouse cursor needs not be visible, so we hide it via the
	irr::IrrlichtDevice::ICursorControl.
	*/
	device->getCursorControl()->setVisible(false);

	/*
	We have done everything, so lets draw it. We also write the current
	frames per second and the primitives drawn into the caption of the
	window. The test for irr::IrrlichtDevice::isWindowActive() is optional,
	but prevents the engine to grab the mouse cursor after task switching
	when other programs are active. The call to
	irr::IrrlichtDevice::yield() will avoid the busy loop to eat up all CPU
	cycles when the window is not active.
	*/
#ifdef USE_BNG_ADD
	video::SMaterial material;
	material.setTexture(0,0);
	material.Lighting = false;
	material.Wireframe = true;
	scene::ISceneNode* highlightedSceneNode = 0;
	scene::ISceneCollisionManager* collMan = smgr->getSceneCollisionManager();
#endif

	int lastFPS = -1;

	//zqf_added
	//@to-do: implement this using OOP
	CreateSocket();

	/*int w = driver->getScreenSize().Width;
	int h = driver->getScreenSize().Height;
	const int LENGTH = w*h*4;
	GLvoid* buffer = new unsigned char[LENGTH];*/

	while(device->run())
	{
		if (device->isWindowActive())
		{
			driver->beginScene(true, true, video::SColor(255,200,200,200));
			smgr->drawAll();

#ifdef USE_BNG_ADD
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


#endif
		driver->endScene();
		printf("write image to file\n");
		video::IImage* image = device->getVideoDriver()->createScreenShot();
		if (image)
		{
			device->getVideoDriver()->writeImageToFile(image, "screenshot.jpg");
			image->drop();
		}
		printf("image write to file done\n");

		const int SIZE = 1024 * 8;
		FILE* fp;
		int send_count;
		int length;

		SOCKET serConn = sock_client;

		printf("open image from file\n");
		if((fp=fopen("screenshot.jpg","rb"))==NULL)
		{
			printf("从服务器端返回文件未打开\n");
		}
		printf("start sending\n");
		fseek(fp,0L,SEEK_END);
		length=ftell(fp);
		printf("待传送文件大小： %d\n",length);

		send(serConn, (char *)&length + 3, 1, 0);
		send(serConn, (char *)&length + 2, 1, 0);
		send(serConn, (char *)&length + 1, 1, 0);
		send(serConn, (char *)&length + 0, 1, 0);
		fseek(fp,0L,SEEK_SET);
		//传送文件
		long int y=0;
		char trans[SIZE];
		while(!feof(fp))
		{

			fread(trans,1,SIZE,fp);
			y=y+SIZE;
			if(y<length)
			{
				send_count=send(serConn,trans,SIZE,0);
				printf("图片字节数：%d\n",send_count); 
			}
			else
			{
				send(serConn,trans,length+SIZE-y,0);
			}
		}
		fclose(fp);
		printf("文件发送完毕\n");

			int fps = driver->getFPS();

			if (lastFPS != fps)
			{
				core::stringw str = L"Irrlicht Engine - Quake 3 Map example [";
				str += driver->getName();
				str += "] FPS:";
				str += fps;

				device->setWindowCaption(str.c_str());
				lastFPS = fps;
			}
		}
		else
			device->yield();
	}

	/*
	In the end, delete the Irrlicht device.
	*/
	device->drop();
	return 0;
}

/*
That's it. Compile and play around with the program.
**/
