#pragma once

#include "global.h"

#include <string>
#include <json/json.h>

RD_NAMESPACE_BEGIN

struct Resource {
	std::string name;
	std::string path;
	std::string description;

	Resource() {};
	Resource(const std::string &json);
	Resource(const Json::Value &value);

	void parseJson(const Json::Value &value);
	Json::Value toJson() const;

	friend inline bool operator==(const Resource &r1, const Resource &r2) { r1.path == r2.path; }
};

RD_NAMESPACE_END
