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
	setDescription(value[3].asString());

	for(Json::Value::iterator i = value[4].begin(); i != value[4].end(); i++)
	{
		addImage((*i).asString());
	}

	for(Json::Value::iterator i = value[5].begin(); i != value[5].end(); i++)
	{
		addAudio((*i).asString());
	}

	for(Json::Value::iterator i = value[6].begin(); i != value[6].end(); i++)
	{
		addVideo((*i).asString());
	}
}

std::string Hotspot::toJson() const
{
	Json::Value value;
	Json::FastWriter writer;

	value[0] = _name;
	
	Json::Value &pos = value[1];
	pos[0] = _pos.X;
	pos[1] = _pos.Y;
	pos[2] = _pos.Z;

	Json::Value &size = value[2];
	size[0] = _size.Width;
	size[1] = _size.Height;

	value[3] = _description;

	Json::Value &image = value[4];
	for(std::list<std::string>::const_iterator i = _image.begin(); i != _image.end(); i++)
	{
		image.append(*i);
	}

	Json::Value &audio = value[5];
	for(std::list<std::string>::const_iterator i = _audio.begin(); i != _audio.end(); i++)
	{
		audio.append(*i);
	}

	Json::Value &video = value[6];
	for(std::list<std::string>::const_iterator i = _video.begin(); i != _video.end(); i++)
	{
		video.append(*i);
	}

	return writer.write(value);
}
