
#include <iostream>
#include "ControlPanel.h"

int main()
{
	ControlPanel cpanel(std::cin, std::cout);
	return cpanel.exec();
}
