#pragma once

#include "global.h"

#include <string>
#include "Resource.h"
#include "irrlicht.h"

RD_NAMESPACE_BEGIN

struct SceneMap : public Resource
{
	irr::core::vector3df cameraPosition;
	irr::core::vector3df cameraTarget;
	std::string hotspotPath;
	std::string meshPath;
	std::string portalPath;

	SceneMap() {};
	SceneMap(const std::string &json);
	SceneMap(const Json::Value &value);

	void parseJson(const Json::Value &value);
	Json::Value toJson() const;
};

RD_NAMESPACE_END
