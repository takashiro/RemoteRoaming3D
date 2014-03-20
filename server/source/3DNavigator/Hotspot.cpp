#include "Hotspot.h"

#include <json/json.h>
#include <fstream>

using namespace irr;

Hotspot::Hotspot()
{
}

std::string Hotspot::toJson() const
{
	Json::Value value;
	Json::FastWriter writer;

	value[0][0] = _pos.x;
	value[0][1] = _pos.y;
	value[0][2] = _pos.z;
	value[1] = _name;

	return writer.write(value);
}

Hotspot Hotspot::FromJson(const std::string &json)
{
	Hotspot hotspot;
	
	Json::Reader reader;
	Json::Value value;
	if(reader.parse(json, value))
	{
		hotspot.setPositionX(value[0][0].asFloat());
		hotspot.setPositionY(value[0][1].asFloat());
		hotspot.setPositionZ(value[0][2].asFloat());
		hotspot.setName(value[1].asString());
	}

	return hotspot;
}

Hotspot::Position::Position()
{
}

Hotspot::Position::Position(const core::vector3df &pos)
{
	x = pos.X;
	y = pos.Y;
	z = pos.Z;
}

Hotspot::Position::operator irr::core::vector3df()
{
	core::vector3df pos;
	pos.X = x;
	pos.Y = y;
	pos.Z = z;
	return pos;
}
