#ifndef __SCENEMAP_H__
#define __SCENEMAP_H__

#include <string>
#include "Resource.h"
#include "irrlicht.h"

struct SceneMap: public Resource{
	irr::core::vector3df camera_position;
	irr::core::vector3df camera_target;
	std::string hotspot_path;
	std::string mesh_path;

	SceneMap(){};
	SceneMap(const std::string &json);
	SceneMap(const Json::Value &value);

	void parseJson(const Json::Value &value);
	Json::Value toJson() const;
};

#endif
