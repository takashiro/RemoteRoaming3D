#include "Resource.h"

RD_NAMESPACE_BEGIN

Resource::Resource(const std::string &json)
{
	Json::Value value;
	Json::Reader reader;
	if (reader.parse(json, value)) {
		parseJson(json);
	}
}

Resource::Resource(const Json::Value &value)
{
	parseJson(value);
}

void Resource::parseJson(const Json::Value &value)
{
	name = value["name"].asString();
	path = value["path"].asString();
	description = value["description"].asString();
}

Json::Value Resource::toJson() const
{
	Json::Value value;
	value.append(name);
	value.append(path);
	value.append(description);
	return value;
}

RD_NAMESPACE_END
