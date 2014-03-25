#ifndef _HOTSPOT_H_
#define _HOTSPOT_H_

#include <irrlicht.h>
#include <json/json.h>
#include <string>
#include <list>

class Hotspot
{
public:
	struct Resource{
		std::string name;
		std::string path;
		std::string description;

		Resource(){};
		Resource(const std::string &json);
		Resource(const Json::Value &value);
		Json::Value toJson() const;
		friend inline bool operator==(const Resource &r1, const Resource &r2){r1.path == r2.path;}
	};

	struct Image: public Resource{
		Image(){};
		Image(const std::string &json);
		Image(const Json::Value &value);
	};

	struct Media: public Resource{
		Media(){};
		Media(const std::string &json);
		Media(const Json::Value &value);

		std::string thumbnail;
		Json::Value toJson() const;
	};

	Hotspot();
	Hotspot(const std::string &json);
	Hotspot(const Json::Value &value);

	Json::Value toJson() const;

	inline std::string getName() const{return _name;}
	inline void setName(const std::string &name){_name = name;}

	inline irr::core::vector3df getPosition() const{return _pos;}
	inline void setPosition(const irr::core::vector3df &pos){_pos = pos;}

	inline irr::core::dimension2d<irr::f32> getSize() const{return _size;}
	inline void setSize(const irr::core::dimension2d<irr::f32> &size){_size = size;}

	inline void setNode(irr::scene::ISceneNode *node){_node = node;}
	inline irr::scene::ISceneNode *getNode() const{return _node;}

	inline std::string getDescription() const{return _description;}
	inline void setDescription(const std::string &description){_description = description;}

	inline void addImage(const Image &image){_image.push_back(image);}
	inline void removeImage(const Image &image){_image.remove(image);}
	inline void addMedia(const Media &media){_media.push_back(media);}
	inline void removeMedia(const Media &media){_media.remove(media);}

protected:
	irr::core::vector3df _pos;
	irr::core::dimension2d<irr::f32> _size;
	std::string _name;
	irr::scene::ISceneNode *_node;
	std::string _description;
	std::list<Image> _image;
	std::list<Media> _media;
};

#endif
