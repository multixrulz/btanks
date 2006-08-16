#ifndef __WORLD_BASE_OBJECT_H__
#define __WORLD_BASE_OBJECT_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <string>
#include "math/v3.h"
#include "mrt/serializable.h"
 
namespace sdlx {
	class Surface;
}

class BaseObject : public mrt::Serializable {
public:
	v3<int> size;
	float mass, speed, ttl, impassability;
	int hp;
	
	bool piercing;
	
	std::string classname;
	
	BaseObject(const std::string &classname);
	virtual ~BaseObject();
	
	virtual void tick(const float dt) = 0;
	virtual void render(sdlx::Surface &surf, const int x, const int y) = 0;
	virtual void emit(const std::string &event, const BaseObject * emitter = NULL);
	
	const float getCollisionTime(const v3<float> &pos, const v3<float> &vel, const float r) const;
	
	const bool isDead() const;
	const int getID() const { return _id; }

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const std::string dump() const;
	void inheritParameters(const BaseObject *other);
	void convertToAbsolute(v3<float> &pos, const v3<float> &dpos);
	
protected:
	int _id;
	void getPosition(v3<float> &position);
	void getPosition(v3<int> &position);
	
	v3<float> _velocity, _old_velocity, _direction;
private:
	bool _dead;
	v3<float> _position;
	int _owner_id;
	friend class IWorld;
};

#endif

