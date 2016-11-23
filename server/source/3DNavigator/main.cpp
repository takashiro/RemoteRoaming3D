
#include <iostream>
#include <string>

#include "ControlPanel.h"

int main()
{
	ControlPanel cpanel(std::cin, std::cout);
	return cpanel.exec();
}
