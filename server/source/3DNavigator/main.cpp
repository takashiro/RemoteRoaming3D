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
To be able to use the Irrlicht.DLL file, we need to link with the
Irrlicht.lib. We could set this option in the project settings, but to make it
easy, we use a pragma comment lib:
*/
#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

#pragma comment(lib, "Json.lib")
#pragma comment(lib,"ws2_32.lib")

/* Ok, lets start. We use the main() method as start, not the WinMain(). */

int main()
{	
	//create a server
	Server server;
	server.setDriverType(irr::video::EDT_OPENGL);
	server.loadSceneMap("scenemap.json");

	ControlPanel cpanel(std::cin, std::cout);
	return cpanel.exec();
}

/*
That's it. Compile and play around with the program.
**/
