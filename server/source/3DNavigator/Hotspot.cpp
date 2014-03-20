#include "Hotspot.h"

#include <fstream>

using namespace irr;

Hotspot::Hotspot()
{
}

Hotspot::Hotspot(const std::string &json)
{
	Json::Reader reader;
	Json::Value value;
	if(reader.parse(json, value))
	{
		Hotspot(value);
	}
}

Hotspot::Hotspot(const Json::Value &value)
{
	setName(value[0].asString());
	setPosition(core::vector3df(value[1][0].asFloat(), value[1][1].asFloat(), value[1][2].asFloat()));
	setSize(core::dimension2d<f32>(value[2][0].asFloat(), value[2][1].asFloat()));
}

std::string Hotspot::toJson() const
{
	Json::Value value;
	Json::FastWriter writer;

	value[0][0] = _pos.X;
	value[0][1] = _pos.Y;
	value[0][2] = _pos.Z;
	value[1] = _name;

	return writer.write(value);
}
