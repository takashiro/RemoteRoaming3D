#pragma once

#include <irrlicht.h>
#include <json/json.h>
#include <string>
#include <list>

#include "Resource.h"

RD_NAMESPACE_BEGIN

class Hotspot
{
public:
	struct Image : public Resource
	{
		Image() {};
		Image(const std::string &json);
		Image(const Json::Value &value);
	};

	struct Media : public Resource
	{
		std::string thumbnail;

		Media() {};
		Media(const std::string &json);
		Media(const Json::Value &value);

		void parseJson(const Json::Value &value);
		Json::Value toJson() const;
	};

	Hotspot();
	Hotspot(const std::string &json);
	Hotspot(const Json::Value &value);

	void parseJson(const Json::Value &value);
	Json::Value toJson() const;

	inline std::string getName() const { return mName; }
	inline void setName(const std::string &name) { mName = name; }

	inline irr::core::vector3df getPosition() const { return mPos; }
	inline void setPosition(const irr::core::vector3df &pos) { mPos = pos; }

	inline irr::core::dimension2d<irr::f32> getSize() const { return mSize; }
	inline void setSize(const irr::core::dimension2d<irr::f32> &size) { mSize = size; }

	inline void setNode(irr::scene::ISceneNode *node) { mNode = node; }
	inline irr::scene::ISceneNode *getNode() const { return mNode; }

	inline std::string getDescription() const { return mDescription; }
	inline void setDescription(const std::string &description) { mDescription = description; }

	inline void addImage(const Image &image) { mImage.push_back(image); }
	inline void removeImage(const Image &image) { mImage.remove(image); }
	inline void addMedia(const Media &media) { mMedia.push_back(media); }
	inline void removeMedia(const Media &media) { mMedia.remove(media); }

protected:
	irr::core::vector3df mPos;
	irr::core::dimension2d<irr::f32> mSize;
	std::string mName;
	irr::scene::ISceneNode *mNode;
	std::string mDescription;
	std::list<Image> mImage;
	std::list<Media> mMedia;
};

RD_NAMESPACE_END
