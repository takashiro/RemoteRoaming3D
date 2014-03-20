#ifndef _HOTSPOT_H_
#define _HOTSPOT_H_

#include <irrlicht.h>
#include <json/json.h>
#include <string>
#include <list>

class Hotspot
{
public:
	Hotspot();
	Hotspot(const std::string &json);
	Hotspot(const Json::Value &value);

	std::string toJson() const;

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

	inline void addImage(const std::string &path){_image.push_back(path);}
	inline void removeImage(const std::string &path){_image.remove(path);}
	inline void addAudio(const std::string &path){_audio.push_back(path);}
	inline void removeAudio(const std::string &path){_audio.remove(path);}
	inline void addVideo(const std::string &path){_video.push_back(path);}
	inline void removeVideo(const std::string &path){_video.remove(path);}

protected:
	irr::core::vector3df _pos;
	irr::core::dimension2d<irr::f32> _size;
	std::string _name;
	irr::scene::ISceneNode *_node;
	std::string _description;
	std::list<std::string> _image, _audio, _video;
};

#endif
