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
		addImage(*i);
	}

	for(Json::Value::iterator i = value[6].begin(); i != value[6].end(); i++)
	{
		addMedia(*i);
	}
}

Json::Value Hotspot::toJson() const
{
	Json::Value value;

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
	for(std::list<Image>::const_iterator i = _image.begin(); i != _image.end(); i++)
	{
		image.append(i->toJson());
	}

	Json::Value &video = value[5];
	for(std::list<Media>::const_iterator i = _media.begin(); i != _media.end(); i++)
	{
		video.append(i->toJson());
	}

	return value;
}

Hotspot::Resource::Resource(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if(reader.parse(json, value))
	{
		Resource(json);
	}
}

Hotspot::Resource::Resource(const Json::Value &value)
{
	name = value[0].asString();
	path = value[1].asString();
	description = value[2].asString();
}

Json::Value Hotspot::Resource::toJson() const{
	Json::Value value;
	value.append(name);
	value.append(path);
	value.append(description);
	return value;
}

Hotspot::Image::Image(const std::string &json)
	:Resource(json)
{
}

Hotspot::Image::Image(const Json::Value &value)
	:Resource(value)
{
}

Hotspot::Media::Media(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if(reader.parse(json, value))
	{
		Media(json);
	}
}

Hotspot::Media::Media(const Json::Value &value)
	:Resource(value)
{
	thumbnail = value[3].asString();
}

Json::Value Hotspot::Media::toJson() const{
	Json::Value value = Resource::toJson();
	value.append(thumbnail);
	return value;
}
