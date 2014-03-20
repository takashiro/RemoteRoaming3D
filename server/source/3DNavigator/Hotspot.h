#ifndef _HOTSPOT_H_
#define _HOTSPOT_H_

#include <irrlicht.h>
#include <json/json.h>
#include <string>

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

protected:
	irr::core::vector3df _pos;
	irr::core::dimension2d<irr::f32> _size;
	std::string _name;
	irr::scene::ISceneNode *_node;
};

#endif
