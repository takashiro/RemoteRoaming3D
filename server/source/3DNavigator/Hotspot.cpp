#include "Hotspot.h"

#include <fstream>

using namespace irr;

RD_NAMESPACE_BEGIN

Hotspot::Hotspot()
{
}

Hotspot::Hotspot(const std::string &json)
{
	Json::Reader reader;
	Json::Value value;
	if (reader.parse(json, value)) {
		Hotspot(value);
	}
}

Hotspot::Hotspot(const Json::Value &value)
{
	setName(value[0].asString());
	setPosition(core::vector3df(value[1][0].asFloat(), value[1][1].asFloat(), value[1][2].asFloat()));
	setSize(core::dimension2d<f32>(value[2][0].asFloat(), value[2][1].asFloat()));
	setDescription(value[3].asString());

	for (const Json::Value &image : value[4]) {
		addImage(image);
	}

	for (const Json::Value &media : value[5]) {
		addMedia(media);
	}
}

Json::Value Hotspot::toJson() const
{
	Json::Value value;

	value[0] = mName;

	Json::Value &pos = value[1];
	pos[0] = mPos.X;
	pos[1] = mPos.Y;
	pos[2] = mPos.Z;

	Json::Value &size = value[2];
	size[0] = mSize.Width;
	size[1] = mSize.Height;

	value[3] = mDescription;

	Json::Value &image = value[4];
	for (const Image &i : mImage) {
		image.append(i.toJson());
	}

	Json::Value &video = value[5];
	for (const Media &m : mMedia) {
		video.append(m.toJson());
	}

	return value;
}

Hotspot::Image::Image(const std::string &json)
	:Resource(json)
{
}

Hotspot::Image::Image(const Json::Value &value)
	: Resource(value)
{
}

Hotspot::Media::Media(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if (reader.parse(json, value)) {
		parseJson(value);
	}
}

Hotspot::Media::Media(const Json::Value &value)
{
	parseJson(value);
}

void Hotspot::Media::parseJson(const Json::Value &value)
{
	Resource::parseJson(value);
	thumbnail = value[3].asString();
}

Json::Value Hotspot::Media::toJson() const
{
	Json::Value value = Resource::toJson();
	value.append(thumbnail);
	return value;
}

RD_NAMESPACE_END
