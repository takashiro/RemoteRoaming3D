#ifndef _HOTSPOT_H_
#define _HOTSPOT_H_

#include <irrlicht.h>
#include <string>

class Hotspot
{
public:
	Hotspot();
	
	struct Position
	{
		float x;
		float y;
		float z;

		Position();
		Position(const irr::core::vector3df &pos);
		operator irr::core::vector3df();
	};

	std::string toJson() const;
	static Hotspot FromJson(const std::string &json);

	inline std::string getName() const{return _name;}
	inline void setName(const std::string &name){_name = name;}

	inline Position getPosition() const{return _pos;}
	inline void setPosition(const Position &pos){_pos = pos;}
	inline void setPositionX(float x){_pos.x = x;}
	inline void setPositionY(float y){_pos.y = y;}
	inline void setPositionZ(float z){_pos.z = z;}

protected:	
	Position _pos;
	std::string _name;
};

#endif
