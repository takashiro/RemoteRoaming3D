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

#include <iostream>
#include <string>

#include "Server.h"
#include "ControlPanel.h"

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

/*
Again, to be able to use the Irrlicht.DLL file, we need to link with the
Irrlicht.lib. We could set this option in the project settings, but to make it
easy, we use a pragma comment lib:
*/
#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

#pragma comment(lib, "Json.lib")
#pragma comment(lib,"ws2_32.lib")

/* Ok, lets start. Again, we use the main() method as start, not the WinMain(). */

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
	irr::video::E_DRIVER_TYPE driverType;

	printf("Please select the driver you want for this example:\n"\
		" (a) OpenGL 1.5\n (b) Direct3D 9.0c\n (c) Direct3D 8.1\n"\
		" (d) Burning's Software Renderer\n (e) Software Renderer\n"\
		" (f) NullDevice\n (otherKey) exit\n$ ");

	char i;
	std::cin >> i;

	switch(i)
	{
		case 'a': driverType = irr::video::EDT_OPENGL;break;
		case 'b': driverType = irr::video::EDT_DIRECT3D9;break;
		case 'c': driverType = irr::video::EDT_DIRECT3D8;break;
		case 'd': driverType = irr::video::EDT_BURNINGSVIDEO;break;
		case 'e': driverType = irr::video::EDT_SOFTWARE;break;
		case 'f': driverType = irr::video::EDT_NULL;break;
		default: return 1;
	}

	//create a server
	Server server;
	ServerInstance = &server;
	server.setDriverType(driverType);
	server.listenTo(6666);
	server.broadcastConfig();

	ControlPanel cpanel(std::cin, std::cout);
	return cpanel.exec();
}

/*
That's it. Compile and play around with the program.
**/
