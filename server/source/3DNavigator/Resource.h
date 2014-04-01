#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <string>
#include <json/json.h>

struct Resource{
	std::string name;
	std::string path;
	std::string description;

	Resource(){};
	Resource(const std::string &json);
	Resource(const Json::Value &value);
		
	void parseJson(const Json::Value &value);
	Json::Value toJson() const;
		
	friend inline bool operator==(const Resource &r1, const Resource &r2){r1.path == r2.path;}
};

#endif
