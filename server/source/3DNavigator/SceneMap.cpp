#include "SceneMap.h"

RD_NAMESPACE_BEGIN

SceneMap::SceneMap(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if (reader.parse(json, value)) {
		parseJson(value);
	}
}

SceneMap::SceneMap(const Json::Value &value)
{
	parseJson(value);
}

void SceneMap::parseJson(const Json::Value &value)
{
	Resource::parseJson(value);

	const Json::Value &pos = value[3];
	cameraPosition.X = pos[0].asFloat();
	cameraPosition.Y = pos[1].asFloat();
	cameraPosition.Z = pos[2].asFloat();

	const Json::Value &look_at = value[4];
	cameraTarget.X = look_at[0].asFloat();
	cameraTarget.Y = look_at[1].asFloat();
	cameraTarget.Z = look_at[2].asFloat();

	hotspotPath = value[5].asString();
	meshPath = value[6].asString();
}

Json::Value SceneMap::toJson() const
{
	Json::Value value = Resource::toJson();

	Json::Value pos;
	pos.append(cameraPosition.X);
	pos.append(cameraPosition.Y);
	pos.append(cameraPosition.Z);
	value.append(pos);

	Json::Value target;
	target.append(cameraTarget.X);
	target.append(cameraTarget.Y);
	target.append(cameraTarget.Z);
	value.append(target);

	value.append(hotspotPath);
	value.append(meshPath);

	return value;
}

RD_NAMESPACE_END
