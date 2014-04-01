#include "Resource.h"

Resource::Resource(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if(reader.parse(json, value))
	{
		parseJson(json);
	}
}

Resource::Resource(const Json::Value &value)
{
	parseJson(value);
}


void Resource::parseJson(const Json::Value &value)
{
	name = value[0].asString();
	path = value[1].asString();
	description = value[2].asString();
}

Json::Value Resource::toJson() const{
	Json::Value value;
	value.append(name);
	value.append(path);
	value.append(description);
	return value;
}
