#include "SceneMap.h"

SceneMap::SceneMap(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if(reader.parse(json, value))
	{
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
	camera_position.X = pos[0].asFloat();
	camera_position.Y = pos[1].asFloat();
	camera_position.Z = pos[2].asFloat();

	const Json::Value &look_at = value[4];
	camera_target.X = look_at[0].asFloat();
	camera_target.Y = look_at[1].asFloat();
	camera_target.Z = look_at[2].asFloat();

	hotspot_path = value[5].asString();
	mesh_path = value[6].asString();
}

Json::Value SceneMap::toJson() const
{
	Json::Value value = Resource::toJson();
	
	Json::Value pos;
	pos.append(camera_position.X);
	pos.append(camera_position.Y);
	pos.append(camera_position.Z);
	value.append(pos);

	Json::Value target;
	target.append(camera_target.X);
	target.append(camera_target.Y);
	target.append(camera_target.Z);
	value.append(target);

	value.append(hotspot_path);
	value.append(mesh_path);
	
	return value;
}
