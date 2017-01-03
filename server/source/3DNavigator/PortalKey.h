#pragma once

#include <irrlicht.h>
#include <string>

struct PortalKey
{
	std::string name;
	irr::core::vector3df pos;
	irr::core::vector3df target;
};
